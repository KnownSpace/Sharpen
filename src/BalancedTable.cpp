#include <sharpen/BalancedTable.hpp>

#include <cstring>
#include <cassert>

#include <sharpen/IteratorOps.hpp>
#include <sharpen/IntOps.hpp>

void sharpen::BalancedTable::InitFile()
{
    sharpen::FilePointer pointers[2];
    std::memset(&pointers,0,sizeof(pointers));
    this->channel_->WriteAsync(reinterpret_cast<char*>(&pointers),sizeof(pointers),0);
    this->offset_ = sizeof(pointers);
}

sharpen::FilePointer sharpen::BalancedTable::AllocMemory(sharpen::Uint64 size)
{
    if(!this->freeArea_.empty())
    {
        sharpen::Size moved{0};
        auto ite = this->freeArea_.end();
        for (auto begin = this->freeArea_.rbegin(),end = this->freeArea_.rend(); begin != end; ++begin)
        {
            moved += 1;
            if(begin->size_ > size)
            {
                ite = sharpen::IteratorBackward(this->freeArea_.end(),moved + 1);
                break;
            }
        }
        if (ite != this->freeArea_.end())
        {
            sharpen::FilePointer pointer{*ite};
            if(moved)
            {
                sharpen::FilePointer next{*sharpen::IteratorForward(ite,1)};
                sharpen::Uint64 prev{0};
                if (ite != this->freeArea_.begin())
                {
                    prev = sharpen::IteratorBackward(ite,1)->offset_;
                }
                this->channel_->WriteAsync(reinterpret_cast<char*>(&next),sizeof(next),prev);
            }
            this->freeArea_.erase(ite);
            return pointer;
        }
    }
    sharpen::Uint64 offset{this->offset_};
    this->offset_ += size;
    sharpen::FilePointer pointer;
    pointer.offset_ = offset;
    pointer.size_ = size;
    return pointer;    
}

void sharpen::BalancedTable::FreeMemory(sharpen::FilePointer pointer)
{
    if(!pointer.size_)
    {
        return;
    }
    assert(pointer.size_ >= sizeof(sharpen::FilePointer));
    sharpen::Uint64 offset{0};
    if(!this->freeArea_.empty())
    {
        offset = this->freeArea_.back().offset_;
    }
    sharpen::FilePointer next;
    std::memset(&next,0,sizeof(next));
    this->channel_->WriteAsync(reinterpret_cast<char*>(&next),sizeof(next),pointer.offset_);
    this->channel_->WriteAsync(reinterpret_cast<char*>(&pointer),sizeof(pointer),offset);
    this->freeArea_.emplace_back(pointer);
}

void sharpen::BalancedTable::InitFreeArea()
{
    sharpen::FilePointer pointer;
    std::memset(&pointer,0,sizeof(pointer));
    this->channel_->ReadAsync(reinterpret_cast<char*>(&pointer),sizeof(pointer),0);
    while(pointer.offset_)
    {
        sharpen::FilePointer next;
        this->channel_->ReadAsync(reinterpret_cast<char*>(&next),sizeof(next),pointer.offset_);
        pointer = next;
        this->freeArea_.emplace_back(pointer);
    }
}

void sharpen::BalancedTable::InitRoot()
{
    sharpen::FilePointer pointer;
    this->channel_->ReadAsync(reinterpret_cast<char*>(&pointer),sizeof(pointer),sizeof(pointer));
    this->rootPointer_ = pointer;
    if(pointer.offset_ && pointer.size_)
    {
        sharpen::ByteBuffer buf{pointer.size_};
        this->channel_->ReadAsync(buf,pointer.offset_);
        this->root_.LoadFrom(buf);
        this->root_.SetBlockSize(sharpen::IntCast<sharpen::Size>(pointer.size_));
    }
}

std::shared_ptr<sharpen::BtBlock> sharpen::BalancedTable::LoadFromCache(sharpen::FilePointer pointer) const
{
    char *keyBegin = reinterpret_cast<char*>(&pointer);
    char *keyEnd = keyBegin + sizeof(pointer);
    std::shared_ptr<sharpen::BtBlock> result{this->caches_.Get(keyBegin,keyEnd)};
    return result;
}

void sharpen::BalancedTable::DeleteFromCache(sharpen::FilePointer pointer)
{
    char *keyBegin = reinterpret_cast<char*>(&pointer);
    char *keyEnd = keyBegin + sizeof(pointer);
    this->caches_.Delete(keyBegin,keyEnd);
}

std::shared_ptr<sharpen::BtBlock> sharpen::BalancedTable::LoadCache(sharpen::FilePointer pointer) const
{
    sharpen::ByteBuffer buf;
    return this->LoadCache(pointer,buf);
}

std::shared_ptr<sharpen::BtBlock> sharpen::BalancedTable::LoadCache(sharpen::FilePointer pointer,sharpen::ByteBuffer &buf) const
{
    char *keyBegin = reinterpret_cast<char*>(&pointer);
    char *keyEnd = keyBegin + sizeof(pointer);
    std::shared_ptr<sharpen::BtBlock> result{this->caches_.Get(keyBegin,keyEnd)};
    if(!result)
    {
        result = this->caches_.GetOrEmplace(keyBegin,keyEnd,this->LoadBlock(pointer.offset_,pointer.size_,buf));
    }
    return result;
}

sharpen::BalancedTable::BalancedTable(sharpen::FileChannelPtr channel,sharpen::BtOption opt)
    :channel_(std::move(channel))
    ,freeArea_()
    ,maxRecordsOfBlock_((std::max)(static_cast<sharpen::Uint16>(3),opt.GetMaxRecordsOfBlock()))
    ,root_(0,this->maxRecordsOfBlock_)
    ,offset_(0)
    ,caches_(opt.GetCacheSize())
{
    this->freeArea_.reserve(64);
    sharpen::Uint64 size{this->channel_->GetFileSize()};
    if(!size)
    {
        this->InitFile();
    }
    else
    {
        this->offset_ = size;
    }
    this->InitFreeArea();
    this->InitRoot();
}

sharpen::BalancedTable::BalancedTable(sharpen::FileChannelPtr channel)
    :BalancedTable(std::move(channel),sharpen::BtOption{})
{}

void sharpen::BalancedTable::WriteRootPointer(sharpen::FilePointer pointer)
{
    this->channel_->WriteAsync(reinterpret_cast<char*>(&pointer),sizeof(pointer),sizeof(pointer));
}

sharpen::FilePointer sharpen::BalancedTable::WriteBlock(sharpen::BtBlock &block,sharpen::FilePointer pointer)
{
    if(block.IsOverflow())
    {
        sharpen::Size oldSize{block.GetBlockSize()};
        sharpen::Uint64 newSize{this->ComputeBlockSize(block)};
        sharpen::FilePointer newPointer{this->AllocMemory(newSize)};
        block.SetBlockSize(newPointer.size_);
        try
        {
            sharpen::ByteBuffer buf{newPointer.size_};
            block.StoreTo(buf);
            this->channel_->WriteAsync(buf,newPointer.offset_);
        }
        catch(const std::exception&)
        {
            block.SetBlockSize(oldSize);
            this->FreeMemory(newPointer);
            throw;
        }
        //set prev's next pointer
        if(block.Prev().offset_ && block.Prev().size_)
        {
            //write file
            this->channel_->WriteAsync(reinterpret_cast<char*>(&newPointer),sizeof(newPointer),block.ComputeNextPointer() + block.Prev().offset_);
            //flush cache
            auto cahce{this->LoadFromCache(block.Prev())};
            if(cahce)
            {
                cahce->Next() = newPointer;
            }
        }
        //set next's prev pointer
        if(block.Next().offset_ && block.Next().size_)
        {
            //write file
            this->channel_->WriteAsync(reinterpret_cast<char*>(&newPointer),sizeof(newPointer),block.ComputePrevPointer() + block.Next().offset_);
            //flush cache
            auto cache{this->LoadFromCache(block.Next())};
            if(cache)
            {
                cache->Prev() = newPointer;
            }
        }
        this->DeleteFromCache(pointer);
        this->FreeMemory(pointer);
        return newPointer;
    }
    sharpen::ByteBuffer buf{pointer.size_};
    block.StoreTo(buf);
    this->channel_->WriteAsync(buf,pointer.offset_);
    return pointer;
}

sharpen::Uint64 sharpen::BalancedTable::ComputeBlockSize(const sharpen::BtBlock &block) noexcept
{
    sharpen::Size used{block.GetUsedSize()};
    sharpen::Size blockSize{used/Self::blockSize_};
    if(used % Self::blockSize_)
    {
        blockSize += 1;
    }
    return blockSize * Self::blockSize_;
}

sharpen::FilePointer sharpen::BalancedTable::InsertRecord(sharpen::BtBlock &block,sharpen::ByteBuffer key,sharpen::ByteBuffer value,sharpen::FilePointer pointer,sharpen::Optional<sharpen::BtBlock> &splitedBlock)
{
    //put key and value to block
    block.Put(std::move(key),std::move(value));
    //split block if we needed
    if (block.GetSize() > this->maxRecordsOfBlock_)
    {
        //init new block
        sharpen::BtBlock nextBlock{block.Split()};
        sharpen::FilePointer nextPointer;
        std::memset(&nextPointer,0,sizeof(nextPointer));
        nextBlock.Next() = block.Next();
        nextBlock.Prev() = pointer;
        //write new block
        nextPointer = this->WriteBlock(nextBlock,nextPointer);
        //set new block next pointer
        block.Next() = nextPointer;
        //rewrite old block
        sharpen::FilePointer oldPointer{this->WriteBlock(block,pointer)};
        //never overflow
        assert(oldPointer.offset_ == pointer.offset_);
        static_cast<void>(oldPointer);
        splitedBlock.Construct(std::move(nextBlock));
        return pointer;
    }
    //write to file
    pointer = this->WriteBlock(block,pointer);
    return pointer;
}

sharpen::BtBlock sharpen::BalancedTable::LoadBlock(sharpen::Uint64 offset,sharpen::Uint64 size,sharpen::ByteBuffer &buf) const
{
    buf.ExtendTo(sharpen::IntCast<sharpen::Size>(size));
    this->channel_->ReadAsync(buf.Data(),size,offset);
    sharpen::BtBlock block{sharpen::IntCast<sharpen::Size>(size)};
    block.LoadFrom(buf);
    return block;
}

sharpen::BtBlock sharpen::BalancedTable::LoadBlock(sharpen::Uint64 offset,sharpen::Uint64 size) const
{
    sharpen::ByteBuffer buf;
    return this->LoadBlock(offset,size,buf);
}

std::pair<sharpen::BtBlock,sharpen::FilePointer> sharpen::BalancedTable::LoadBlockAndPointer(const sharpen::ByteBuffer &key) const
{
    sharpen::Size depth{this->GetDepth()};
    if(depth)
    {
        auto ite = this->root_.Find(key);
        sharpen::ByteBuffer buf;
        sharpen::BtBlock block;
        sharpen::FilePointer pointer;
        std::memset(&pointer,0,sizeof(pointer));
        std::shared_ptr<sharpen::BtBlock> blockRef{nullptr};
        for (sharpen::Size i = 0,count = depth; i != count; ++i)
        {
            pointer = ite->ValueAsPointer();
            blockRef = this->LoadFromCache(pointer);
            if(!blockRef)
            {
                block = this->LoadBlock(pointer.offset_,pointer.size_,buf);
                ite = block.Find(key);
            }
            else
            {
                ite = blockRef->Find(key);
            }
        }
        return std::make_pair(std::move(block),pointer);
    }
    return std::make_pair(this->root_,this->rootPointer_);
}

sharpen::BtBlock sharpen::BalancedTable::LoadBlock(const sharpen::ByteBuffer &key) const
{
    return this->LoadBlockAndPointer(key).first;
}

std::vector<std::pair<std::shared_ptr<sharpen::BtBlock>,sharpen::FilePointer>> sharpen::BalancedTable::GetPath(const sharpen::ByteBuffer &key,bool doCache) const
{
    sharpen::Size depth{this->GetDepth()};
    std::vector<std::pair<std::shared_ptr<sharpen::BtBlock>,sharpen::FilePointer>> path;
    if(depth)
    {
        auto ite = this->root_.Find(key);
        path.reserve(depth);
        sharpen::ByteBuffer buf;
        for (sharpen::Size i = 0,count = depth; i != count; ++i)
        {
            sharpen::FilePointer pointer{ite->ValueAsPointer()};
            std::shared_ptr<sharpen::BtBlock> block{nullptr};
            if(doCache)
            {
                block = this->LoadCache(pointer);
            }
            else
            {
                block = this->LoadFromCache(pointer);
                if(!block)
                {
                    block = std::make_shared<sharpen::BtBlock>(this->LoadBlock(pointer.offset_,pointer.size_,buf));
                }
            }
            path.emplace_back(std::move(block),pointer);
            ite = path.back().first->Find(key);
        }
    }
    return path;
}

void sharpen::BalancedTable::InsertToRoot(sharpen::ByteBuffer key,sharpen::ByteBuffer value)
{
    sharpen::Optional<sharpen::BtBlock> splitedBlock;
    sharpen::FilePointer pointer{this->InsertRecord(this->root_,std::move(key),std::move(value),this->rootPointer_,splitedBlock)};
    if(this->rootPointer_.offset_ != pointer.offset_)
    {
        this->rootPointer_ = pointer;
        this->WriteRootPointer(this->rootPointer_);
    }
    if(splitedBlock.Exist())
    {
        //we need a new root
        sharpen::BtBlock newRoot{0};
        //increase depth
        newRoot.SetDepth(this->GetDepth() + 1);
        //set path
        sharpen::ByteBuffer buf{sizeof(sharpen::FilePointer)};
        std::memcpy(buf.Data(),&this->rootPointer_,sizeof(this->rootPointer_));
        newRoot.Put(this->root_.Begin()->GetKey(),std::move(buf));
        buf.ExtendTo(sizeof(this->rootPointer_));
        std::memcpy(buf.Data(),&this->root_.Next(),sizeof(this->root_.Next()));
        newRoot.Put(splitedBlock.Get().Begin()->GetKey(),std::move(buf));
        //write new root
        std::memset(&pointer,0,sizeof(pointer));
        pointer = this->WriteBlock(newRoot,pointer);
        //write root pointer
        this->rootPointer_ = pointer;
        this->root_ = std::move(newRoot);
        this->WriteRootPointer(this->rootPointer_);
    }
}

void sharpen::BalancedTable::Put(sharpen::ByteBuffer key,sharpen::ByteBuffer value)
{
    //if depth == 0
    //we only have root
    if (!this->GetDepth())
    {
        return this->InsertToRoot(std::move(key),std::move(value));
    }
    //find path
    auto path{this->GetPath(key)};
    std::shared_ptr<sharpen::BtBlock> lastBlock{std::move(path.back().first)};
    sharpen::Optional<sharpen::BtBlock> splitedBlock;
    try
    {
        path.back().second = this->InsertRecord(*lastBlock,std::move(key),std::move(value),path.back().second,splitedBlock);
    }
    catch(const std::exception&)
    {
        //we must flush cache
        //if we failed
        this->DeleteFromCache(path.back().second);
        throw;
    }
    while (splitedBlock.Exist())
    {
        sharpen::ByteBuffer next{sizeof(lastBlock->Next())};
        std::memcpy(next.Data(),&lastBlock->Next(),sizeof(lastBlock->Next()));
        path.pop_back();
        sharpen::BtBlock splited{std::move(splitedBlock.Get())};
        splitedBlock.Reset();
        if(!path.empty())
        {
            lastBlock = std::move(path.back().first);
            path.back().second = this->InsertRecord(*lastBlock,std::move(*splited.Begin()).MoveKey(),std::move(next),path.back().second,splitedBlock);
        }
        else
        {
            this->InsertToRoot(std::move(*splited.Begin()).MoveKey(),std::move(next));
            return;
        }
    }
}

sharpen::Optional<sharpen::ByteBuffer> sharpen::BalancedTable::TryGet(const sharpen::ByteBuffer &key) const
{
    sharpen::BtBlock block{this->LoadBlock(key)};
    auto ite = block.Find(key);
    if(ite != block.End() && ite->GetKey() == key)
    {
        return ite->Value();
    }
    return sharpen::EmptyOpt;
}

sharpen::ByteBuffer sharpen::BalancedTable::Get(const sharpen::ByteBuffer &key) const
{
    sharpen::Optional<sharpen::ByteBuffer> opt{this->TryGet(key)};
    if(!opt.Exist())
    {
        throw std::out_of_range("key doesn't exists");
    }
    return opt.Get();
}

sharpen::ExistStatus sharpen::BalancedTable::Exist(const sharpen::ByteBuffer &key) const
{
    sharpen::BtBlock block{this->LoadBlock(key)};
    auto ite = block.Find(key);
    if(ite != block.End() && ite->GetKey() == key)
    {
        return sharpen::ExistStatus::Exist;
    }
    return sharpen::ExistStatus::NotExist;
}

void sharpen::BalancedTable::DeleteFromRoot(const sharpen::ByteBuffer &key)
{
    this->root_.Delete(key);
    this->WriteBlock(this->root_,this->rootPointer_);
}

void sharpen::BalancedTable::Delete(const sharpen::ByteBuffer &key)
{
    if(!this->GetDepth())
    {
        return this->DeleteFromRoot(key);
    }
    auto path{this->GetPath(key)};
    std::shared_ptr<sharpen::BtBlock> lastBlock{std::move(path.back().first)};
    auto ite = lastBlock->Find(key);
    if(ite == lastBlock->End() || ite->GetKey() != key)
    {
        return;
    }
    if(lastBlock->GetSize() != 1)
    {
        lastBlock->Delete(key);
        this->WriteBlock(*lastBlock,path.back().second);
        return;
    }
    sharpen::ByteBuffer deletedKey;
    while (lastBlock->GetSize() == 1)
    {
        deletedKey = std::move(*lastBlock->Begin()).MoveKey();
        sharpen::FilePointer pointer{path.back().second};
        this->DeleteFromCache(pointer);
        sharpen::FilePointer prev;
        sharpen::FilePointer next;
        std::memcpy(&next,&lastBlock->Next(),sizeof(next));
        std::memcpy(&prev,&lastBlock->Prev(),sizeof(prev));
        if (prev.offset_ && prev.size_)
        {
            this->channel_->WriteAsync(reinterpret_cast<char*>(&next),sizeof(next),lastBlock->ComputeNextPointer() + prev.offset_);
        }
        if(next.offset_ && next.size_)
        {
            this->channel_->WriteAsync(reinterpret_cast<char*>(&prev),sizeof(prev),lastBlock->ComputePrevPointer() + next.offset_);
        }
        path.pop_back();
        this->FreeMemory(pointer);
        if(!path.empty())
        {
            lastBlock = std::move(path.back().first);
        }
        else
        {
            this->root_.FuzzingDelete(key);
            if(this->root_.GetSize() == 1)
            {
                assert(this->root_.Begin()->Value().GetSize() == sizeof(pointer));
                std::memcpy(&pointer,this->root_.Begin()->Value().Data(),sizeof(pointer));
                std::swap(this->rootPointer_,pointer);
                this->WriteRootPointer(this->rootPointer_);
                //we never put root to caches
                this->FreeMemory(pointer);
                this->root_ = this->LoadBlock(this->rootPointer_.offset_,this->rootPointer_.size_);
                return;
            }
            this->WriteBlock(this->root_,this->rootPointer_);
            return;
        }
    }
    lastBlock->FuzzingDelete(deletedKey);
    this->WriteBlock(*lastBlock,path.back().second);
}

std::shared_ptr<const sharpen::BtBlock> sharpen::BalancedTable::FindBlock(const sharpen::ByteBuffer &key,bool doCache) const
{
    sharpen::Size depth{this->GetDepth()};
    if(depth)
    {
        auto ite = this->root_.Find(key);
        std::shared_ptr<sharpen::BtBlock> block{nullptr};
        sharpen::ByteBuffer buf;
        for (sharpen::Size i = 0,count = depth; i != count; ++i)
        {
            sharpen::FilePointer pointer{ite->ValueAsPointer()};
            if(doCache)
            {
                block = this->LoadCache(pointer);
            }
            else
            {
                block = this->LoadFromCache(pointer);
                if(!block)
                {
                    block = std::make_shared<sharpen::BtBlock>(this->LoadBlock(pointer.offset_,pointer.size_,buf));
                }
            }
            ite = block->Find(key);
        }
        return block;
    }
    return std::make_shared<sharpen::BtBlock>(this->root_);
}

std::shared_ptr<const sharpen::BtBlock> sharpen::BalancedTable::FindBlockFromCache(const sharpen::ByteBuffer &key) const
{
    sharpen::Size depth{this->GetDepth()};
    if(depth)
    {
        auto ite = this->root_.Find(key);
        std::shared_ptr<sharpen::BtBlock> block{nullptr};
        for (sharpen::Size i = 0,count = depth; i != count; ++i)
        {
            sharpen::FilePointer pointer{ite->ValueAsPointer()};
            block = this->LoadFromCache(pointer);
            if(!block)
            {
                return nullptr;
            }
            ite = block->Find(key);
        }
        return block;
    }
    return std::make_shared<sharpen::BtBlock>(this->root_);
}

bool sharpen::BalancedTable::IsFault() const
{
    //root never has next pointer
    if(this->root_.Next().offset_)
    {
        return true;
    }
    sharpen::BtBlock leaf{this->root_};
    sharpen::Size depth{this->GetDepth()};
    if(depth)
    {
        sharpen::FilePointer prevPointer;
        for (sharpen::Size i = 0; i != depth; ++i)
        {
            auto it = leaf.Begin();
            assert(it != leaf.End());
            prevPointer = it->ValueAsPointer();
            leaf = this->LoadBlock(prevPointer);
        }
        while (leaf.Next().offset_)
        {
            sharpen::FilePointer nextPointer{leaf.Next()};
            sharpen::BtBlock block{this->LoadBlock(nextPointer)};
            for (auto begin = block.Begin(),end = block.End(); begin != end; begin++)
            {
                //we lost this block
                //it must be fault
                if (this->Exist(begin->GetKey()) == sharpen::ExistStatus::NotExist)
                {
                    return true;
                }
            }
            leaf = std::move(block);
        }
    }
    return false;
}