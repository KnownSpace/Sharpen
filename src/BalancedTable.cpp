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
    }
}

sharpen::BalancedTable::BalancedTable(sharpen::FileChannelPtr channel,sharpen::Uint16 maxRecordOfBlock)
    :channel_(std::move(channel))
    ,freeArea_()
    ,maxRecordsOfBlock_((std::max)(static_cast<sharpen::Uint16>(3),maxRecordOfBlock))
    ,root_(0,this->maxRecordsOfBlock_)
    ,offset_(0)
{
    this->freeArea_.reserve(64);
    sharpen::Uint64 size{this->channel_->GetFileSize()};
    if(!size)
    {
        this->InitFile();
    }
    this->InitFreeArea();
    this->InitRoot();
}

sharpen::BalancedTable::BalancedTable(sharpen::FileChannelPtr channel)
    :BalancedTable(std::move(channel),32)
{}

void sharpen::BalancedTable::WriteRootPointer(sharpen::FilePointer pointer)
{
    this->channel_->WriteAsync(reinterpret_cast<char*>(&pointer),sizeof(pointer),sizeof(pointer));
}

sharpen::FilePointer sharpen::BalancedTable::WriteEndOfBlock(sharpen::BtBlock &block,sharpen::Uint64 offset,sharpen::FilePointer pointer)
{
    if(block.IsOverflow())
    {
        return this->WriteBlock(block,pointer);
    }
    sharpen::ByteBuffer buf;
    block.ReverseBegin()->StoreTo(buf);
    sharpen::Uint16 keyCount{sharpen::IntCast<sharpen::Uint16>(block.GetSize())};
    this->channel_->WriteAsync(reinterpret_cast<char*>(&keyCount),sizeof(keyCount),pointer.offset_ + block.ComputeCounterPointer());
    this->channel_->WriteAsync(buf,offset + pointer.offset_);
    return pointer;
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
            this->channel_->WriteAsync(reinterpret_cast<char*>(&newPointer),sizeof(newPointer),block.ComputeNextPointer() + block.Prev().offset_);
        }
        //set next's prev pointer
        if(block.Next().offset_ && block.Next().size_)
        {
            this->channel_->WriteAsync(reinterpret_cast<char*>(&newPointer),sizeof(newPointer),block.ComputePrevPointer() + block.Next().offset_);
        }
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
    auto tag = block.QueryPutTage(key);
    sharpen::Size offset{0};
    if(tag == sharpen::BtBlock::PutTage::Append)
    {
        offset = block.GetAppendPointer();
    }
    else if (tag == sharpen::BtBlock::PutTage::MotifyEnd)
    {
        offset = block.ComputeMotifyEndPointer();
    }
    block.Put(std::move(key),std::move(value));
    //split block
    if (block.GetSize() > this->maxRecordsOfBlock_)
    {
        sharpen::BtBlock nextBlock{block.Split()};
        sharpen::FilePointer nextPointer;
        nextPointer.offset_ = 0;
        nextPointer.size_ = 0;
        nextBlock.Next() = block.Next();
        nextBlock.Prev() = pointer;
        //write block
        nextPointer = this->WriteBlock(nextBlock,nextPointer);
        //set new block next pointer
        block.Next() = nextPointer;
        //rewrite old block
        sharpen::FilePointer oldPointer{this->WriteBlock(block,pointer)};
        assert(oldPointer.offset_ == pointer.offset_);
        static_cast<void>(oldPointer);
        splitedBlock.Construct(std::move(nextBlock));
        return pointer;
    }
    if(tag == sharpen::BtBlock::PutTage::Normal)
    {
        pointer = this->WriteBlock(block,pointer);
    }
    else
    {
        pointer = this->WriteEndOfBlock(block,offset,pointer);
    }
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
    auto ite = this->root_.Find(key);
    sharpen::Size depth{this->GetDepth()};
    if(depth)
    {
        sharpen::ByteBuffer buf;
        sharpen::BtBlock block;
        sharpen::FilePointer pointer;
        std::memset(&pointer,0,sizeof(pointer));
        for (sharpen::Size i = 0,count = depth; i != count; ++i)
        {
            assert(sizeof(pointer) == ite->Value().GetSize());
            std::memcpy(&pointer,ite->Value().Data(),sizeof(pointer));   
            block = this->LoadBlock(pointer.offset_,pointer.size_,buf);
            ite = block.Find(key);
        }
        return std::make_pair(std::move(block),pointer);
    }
    return std::make_pair(this->root_,this->rootPointer_);
}

sharpen::BtBlock sharpen::BalancedTable::LoadBlock(const sharpen::ByteBuffer &key) const
{
    return this->LoadBlockAndPointer(key).first;
}

std::vector<std::pair<sharpen::BtBlock,sharpen::FilePointer>> sharpen::BalancedTable::GetPath(const sharpen::ByteBuffer &key) const
{
    sharpen::Size depth{this->GetDepth()};
    std::vector<std::pair<sharpen::BtBlock,sharpen::FilePointer>> path;
    if(depth)
    {
        auto ite = this->root_.Find(key);
        path.reserve(depth);
        sharpen::ByteBuffer buf;
        for (sharpen::Size i = 0,count = depth; i != count; ++i)
        {
            sharpen::FilePointer pointer;
            assert(sizeof(pointer) == ite->Value().GetSize());
            std::memcpy(&pointer,ite->Value().Data(),sizeof(pointer));   
            sharpen::BtBlock block{this->LoadBlock(pointer.offset_,pointer.size_,buf)};
            path.emplace_back(std::move(block),pointer);
            ite = path.back().first.Find(key);
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
        sharpen::BtBlock newRoot{0};
        newRoot.SetDepth(this->GetDepth() + 1);
        sharpen::ByteBuffer buf{sizeof(sharpen::FilePointer)};
        std::memcpy(buf.Data(),&this->rootPointer_,sizeof(this->rootPointer_));
        newRoot.Put(this->root_.Begin()->GetKey(),std::move(buf));
        buf.ExtendTo(sizeof(this->rootPointer_));
        std::memcpy(buf.Data(),&this->root_.Next(),sizeof(this->root_.Next()));
        newRoot.Put(splitedBlock.Get().Begin()->GetKey(),std::move(buf));
        std::memset(&pointer,0,sizeof(pointer));
        pointer = this->WriteBlock(newRoot,pointer);
        this->rootPointer_ = pointer;
        this->root_ = std::move(newRoot);
        this->WriteRootPointer(this->rootPointer_);
    }
}

void sharpen::BalancedTable::Put(sharpen::ByteBuffer key,sharpen::ByteBuffer value)
{
    if (!this->GetDepth())
    {
        return this->InsertToRoot(std::move(key),std::move(value));
    }
    auto path{this->GetPath(key)};
    sharpen::BtBlock *lastBlock = &path.back().first;
    sharpen::Optional<sharpen::BtBlock> splitedBlock;
    path.back().second = this->InsertRecord(*lastBlock,std::move(key),std::move(value),path.back().second,splitedBlock);
    while (splitedBlock.Exist())
    {
        sharpen::ByteBuffer next{sizeof(lastBlock->Next())};
        std::memcpy(next.Data(),&lastBlock->Next(),sizeof(lastBlock->Next()));
        path.pop_back();
        sharpen::BtBlock splited{std::move(splitedBlock.Get())};
        splitedBlock.Reset();
        if(!path.empty())
        {
            lastBlock = &path.back().first;
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
        throw std::out_of_range("key doesn't exist");
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
    sharpen::BtBlock *lastBlock = &path.back().first;
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
        sharpen::FilePointer pointer{path.back().second};
        path.pop_back();
        this->FreeMemory(pointer);
        if(!path.empty())
        {
            lastBlock = &path.back().first;
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