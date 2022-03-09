#include <sharpen/BalancedTable.hpp>

#include <cstring>
#include <cassert>

#include <sharpen/IteratorOps.hpp>
#include <sharpen/IntOps.hpp>

sharpen::Int32 sharpen::BalancedTable::CompKey(const sharpen::ByteBuffer &left,const sharpen::ByteBuffer &right) const noexcept
{
    if(this->root_.GetComparator())
    {
        return this->root_.GetComparator()(left,right);
    }
    return left.CompareWith(right);
}

void sharpen::BalancedTable::InitFile()
{
    sharpen::FilePointer pointers[2];
    std::memset(&pointers,0,sizeof(pointers));
    this->channel_->WriteAsync(reinterpret_cast<char*>(&pointers),sizeof(pointers),0);
    this->offset_ = sizeof(pointers);
}

sharpen::FilePointer sharpen::BalancedTable::AllocMemory(sharpen::Uint64 size)
{
    std::unique_lock<sharpen::AsyncMutex> lock{*this->allocLock_};
    if(!this->freeArea_.empty())
    {
        auto ite = this->freeArea_.begin();
        while (ite != this->freeArea_.end())
        {
            if(ite->size_ >= size)
            {
                break;
            }
            ++ite;
        }
        if (ite != this->freeArea_.end())
        {
            sharpen::FilePointer pointer{*ite};
            auto nextIte = sharpen::IteratorForward(ite,1);
            sharpen::FilePointer next;
            std::memset(&next,0,sizeof(next));
            if(nextIte != this->freeArea_.end())
            {
                next = *nextIte;
            }
            sharpen::FilePointer prev;
            std::memset(&prev,0,sizeof(prev));
            prev.size_ = sizeof(sharpen::FilePointer);
            if(ite != this->freeArea_.begin())
            {
                auto prevIte = sharpen::IteratorBackward(ite,1);
                prev = *prevIte;
            }
            this->channel_->WriteAsync(reinterpret_cast<char*>(&next),sizeof(next),prev.offset_);
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
    std::unique_lock<sharpen::AsyncMutex> lock{*this->allocLock_};
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
    while(pointer.offset_ && pointer.offset_ < this->offset_)
    {
        sharpen::FilePointer next;
        std::memset(&next,0,sizeof(next));
        this->channel_->ReadAsync(reinterpret_cast<char*>(&next),sizeof(next),pointer.offset_);
        pointer = next;
        if(pointer.size_)
        {
            this->freeArea_.emplace_back(pointer);
        }
    }
}

void sharpen::BalancedTable::InitRoot()
{
    sharpen::FilePointer pointer;
    this->channel_->ReadAsync(reinterpret_cast<char*>(&pointer),sizeof(pointer),sizeof(pointer));
    if(pointer.offset_ && pointer.size_)
    {
        this->root_ = this->LoadBlock(pointer);
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

sharpen::BalancedTable::BalancedTable(sharpen::FileChannelPtr channel,const sharpen::BtOption &opt)
    :channel_(std::move(channel))
    ,freeArea_()
    ,maxRecordsOfBlock_((std::max)(static_cast<sharpen::Uint16>(3),opt.GetMaxRecordsOfBlock()))
    ,root_(0,this->maxRecordsOfBlock_)
    ,offset_(0)
    ,caches_(opt.GetCacheSize())
    ,lockTable_()
    ,allocLock_(new sharpen::AsyncMutex())
{
    if (!this->allocLock_)
    {
        throw std::bad_alloc();
    }
    this->freeArea_.reserve(64);
    sharpen::Uint64 size{this->channel_->GetFileSize()};
    if(!size)
    {
        this->InitFile();
    }
    else
    {
        if(size < sizeof(sharpen::FilePointer)*2)
        {
            throw std::invalid_argument("not a balance table file");
        }
        this->offset_ = size;
    }
    this->InitFreeArea();
    this->InitRoot();
    //set comparator
    this->root_.SetComparator(opt.GetComparator());
}

sharpen::BalancedTable::BalancedTable(sharpen::FileChannelPtr channel)
    :BalancedTable(std::move(channel),sharpen::BtOption{})
{}

sharpen::BalancedTable &sharpen::BalancedTable::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->channel_ = std::move(other.channel_);
        this->freeArea_ = std::move(other.freeArea_);
        this->maxRecordsOfBlock_ = other.maxRecordsOfBlock_;
        this->root_ = std::move(other.root_);
        this->offset_ = other.offset_;
        this->caches_ = std::move(other.caches_);
        this->lockTable_ = std::move(other.lockTable_);
        this->allocLock_ = std::move(other.allocLock_);
        other.maxRecordsOfBlock_ = 0;
        other.offset_ = 0;
    }
    return *this;
}

void sharpen::BalancedTable::WriteRootPointer(sharpen::FilePointer pointer)
{
    this->channel_->WriteAsync(reinterpret_cast<char*>(&pointer),sizeof(pointer),sizeof(pointer));
}

sharpen::FilePointer sharpen::BalancedTable::AllocMemory(const sharpen::BtBlock &block)
{
    sharpen::Uint64 size{this->ComputeBlockSize(block)};
    return this->AllocMemory(size);
}

void sharpen::BalancedTable::FreeMemory(const sharpen::BtBlock &block)
{
    if (block.GetSwitzzPointer())
    {
        sharpen::FilePointer pointer;
        pointer.offset_ = block.GetSwitzzPointer();
        pointer.size_ = block.GetBlockSize();
        this->FreeMemory(pointer);   
    }
}

void sharpen::BalancedTable::LockBlockForWrite(const sharpen::BtBlock &block) const
{
    assert(block.GetSwitzzPointer() != 0);
    this->lockTable_.GetLock(block.GetSwitzzPointer()).LockWrite();
}

void sharpen::BalancedTable::LockBlockForRead(const sharpen::BtBlock &block) const
{
    assert(block.GetSwitzzPointer() != 0);
    this->lockTable_.GetLock(block.GetSwitzzPointer()).LockRead();
}

void sharpen::BalancedTable::UnlockBlock(const sharpen::BtBlock &block) const noexcept
{
    assert(block.GetSwitzzPointer() != 0);
    this->lockTable_.GetLock(block.GetSwitzzPointer()).Unlock();
}

void sharpen::BalancedTable::WriteBlock(const sharpen::BtBlock &block)
{
    assert(block.GetSwitzzPointer() != 0);
    sharpen::ByteBuffer buf{block.GetBlockSize()};
    block.StoreTo(buf);
    this->channel_->WriteAsync(buf,block.GetSwitzzPointer());
}

void sharpen::BalancedTable::AllocAndWriteBlock(sharpen::BtBlock &block)
{
    sharpen::FilePointer old;
    std::memset(&old,0,sizeof(old));
    if (block.IsOverflow())
    {
        sharpen::FilePointer p;
        p = this->AllocMemory(block);
        old.offset_ = block.GetSwitzzPointer();
        old.size_ = block.GetBlockSize();
        block.SetSwitzzPointer(p.offset_);
        block.SetBlockSize(p.size_);
    }
    //write block
    this->WriteBlock(block);
    if(old.offset_)
    {
        sharpen::FilePointer p;
        p.offset_ = block.GetSwitzzPointer();
        p.size_ = block.GetBlockSize();
        if(block.Prev().offset_)
        {
            this->channel_->WriteAsync(reinterpret_cast<char*>(&p),sizeof(p),block.Prev().offset_ + block.ComputeNextPointer());
            auto prev{this->LoadFromCache(block.Prev())};
            if(prev)
            {
                prev->Next() = p;
            }
        }
        if(block.Next().offset_)
        {
            this->channel_->WriteAsync(reinterpret_cast<char*>(&p),sizeof(p),block.Next().offset_ + block.ComputePrevPointer());
            auto next{this->LoadFromCache(block.Next())};
            if(next)
            {
                next->Prev() = p;
            }
        }
        this->DeleteFromCache(old);
        this->FreeMemory(old);
    }
}

sharpen::Uint64 sharpen::BalancedTable::ComputeBlockSize(const sharpen::BtBlock &block) noexcept
{
    sharpen::Size used{block.GetUsedSize()};
    sharpen::Size blockSize{used/Self::blockSize_};
    if(used % Self::blockSize_)
    {
        blockSize += 1;
    }
    //sure index block never be overflow
    return blockSize * Self::blockSize_;
}

sharpen::BtBlock sharpen::BalancedTable::LoadBlock(sharpen::Uint64 offset,sharpen::Uint64 size,sharpen::ByteBuffer &buf) const
{
    buf.ExtendTo(sharpen::IntCast<sharpen::Size>(size));
    this->channel_->ReadAsync(buf.Data(),size,offset);
    sharpen::BtBlock block{sharpen::IntCast<sharpen::Size>(size)};
    block.LoadFrom(buf);
    //set comparator
    block.SetComparator(this->root_.GetComparator());
    //set switzz pointer
    block.SetSwitzzPointer(offset);
    //set block size
    block.SetBlockSize(sharpen::IntCast<sharpen::Size>(size));
    return block;
}

sharpen::BtBlock sharpen::BalancedTable::LoadBlock(sharpen::Uint64 offset,sharpen::Uint64 size) const
{
    sharpen::ByteBuffer buf;
    return this->LoadBlock(offset,size,buf);
}

sharpen::BtBlock sharpen::BalancedTable::LoadBlock(const sharpen::ByteBuffer &key) const
{
    sharpen::Size depth{this->GetDepth()};
    if(depth)
    {
        auto ite = this->root_.FuzzingFind(key);
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
                assert(block.GetComparator() == this->root_.GetComparator());
                ite = block.FuzzingFind(key);
            }
            else
            {
                ite = blockRef->FuzzingFind(key);
                assert(blockRef->GetComparator() == this->root_.GetComparator());
            }
        }
        if(blockRef)
        {
            //loc block(S)
            this->LockBlockForRead(*blockRef);
            std::unique_lock<sharpen::AsyncReadWriteLock> lock{this->GetBlockLock(*blockRef),std::adopt_lock};
            return *blockRef;
        }
        return block;
    }
    return this->root_;
}

std::vector<std::shared_ptr<sharpen::BtBlock>> sharpen::BalancedTable::GetPath(const sharpen::ByteBuffer &key,bool doCache) const
{
    sharpen::Size depth{this->GetDepth()};
    std::vector<std::shared_ptr<sharpen::BtBlock>> path;
    if(depth)
    {
        auto ite = this->root_.FuzzingFind(key);
        path.reserve(depth);
        sharpen::ByteBuffer buf;
        for (sharpen::Size i = 0,count = depth; i != count; ++i)
        {
            sharpen::FilePointer pointer{ite->ValueAsPointer()};
            std::shared_ptr<sharpen::BtBlock> block{nullptr};
            if(doCache)
            {
                block = this->LoadCache(pointer);
                assert(block->GetComparator() == this->root_.GetComparator());
            }
            else
            {
                block = this->LoadFromCache(pointer);
                if(!block)
                {
                    block = std::make_shared<sharpen::BtBlock>(this->LoadBlock(pointer.offset_,pointer.size_,buf));
                }
                assert(block->GetComparator() == this->root_.GetComparator());
            }
            path.emplace_back(std::move(block));
            ite = path.back()->FuzzingFind(key);
        }
    }
    return path;
}

sharpen::AsyncReadWriteLock &sharpen::BalancedTable::GetRootLock() const
{
    return this->lockTable_.GetLock(0);
}

sharpen::AsyncReadWriteLock &sharpen::BalancedTable::GetBlockLock(const sharpen::BtBlock &block) const
{
    assert(block.GetSwitzzPointer() != 0);
    return this->lockTable_.GetLock(block.GetSwitzzPointer());
}

sharpen::FilePointer sharpen::BalancedTable::GetSwitzzPointer(const sharpen::BtBlock &block) noexcept
{
    sharpen::FilePointer pointer;
    pointer.offset_ = block.GetSwitzzPointer();
    pointer.size_ = block.GetBlockSize();
    return pointer;
}

void sharpen::BalancedTable::PutToRoot(sharpen::ByteBuffer key,sharpen::ByteBuffer value)
{
    //put to root
    this->root_.Put(std::move(key),std::move(value));
    sharpen::Optional<sharpen::BtBlock> overBlock;
    sharpen::FilePointer oldPointer{this->GetSwitzzPointer(this->root_)};
    //if size > max count
    //split the root
    if(this->root_.GetSize() > this->maxRecordsOfBlock_)
    {
        //split block
        overBlock.Construct(this->root_.Split());
        auto &over{overBlock.Get()};
        //write over block
        this->AllocAndWriteBlock(over);
        sharpen::ByteBuffer buf{sizeof(sharpen::FilePointer)};
        sharpen::FilePointer pointer{this->GetSwitzzPointer(over)};
        this->root_.Next() = pointer;
        //write old root
        this->AllocAndWriteBlock(this->root_);
        //create a new root
        sharpen::BtBlock newRoot;
        newRoot.SetDepth(this->root_.GetDepth() + 1);
        newRoot.SetComparator(this->root_.GetComparator());
        //put two blocks to new root
        pointer = this->GetSwitzzPointer(this->root_);
        std::memcpy(buf.Data(),&pointer,sizeof(pointer));
        newRoot.Put(std::move(*this->root_.Begin()).MoveKey(),std::move(buf));
        pointer = this->GetSwitzzPointer(over);
        buf.ExtendTo(sizeof(pointer));
        std::memcpy(buf.Data(),&pointer,sizeof(pointer));
        newRoot.Put(std::move(*over.Begin()).MoveKey(),std::move(buf));
        //write new root
        this->AllocAndWriteBlock(newRoot);
        pointer = this->GetSwitzzPointer(newRoot);
        this->root_ = std::move(newRoot);
        //set root pointer
        this->WriteRootPointer(pointer);
        return;
    }
    this->AllocAndWriteBlock(this->root_);
    if(oldPointer.offset_ != this->root_.GetSwitzzPointer())
    {
        oldPointer = this->GetSwitzzPointer(this->root_);
        this->WriteRootPointer(oldPointer);
    }
}

void sharpen::BalancedTable::Put(sharpen::ByteBuffer key,sharpen::ByteBuffer value)
{
    //lock root(S)
    this->GetRootLock().LockRead();
    std::unique_lock<sharpen::AsyncReadWriteLock> lock{this->GetRootLock(),std::adopt_lock};
    //if we only have root
    if(!this->GetDepth())
    {
        //lock root(S -> X)
        this->GetRootLock().UpgradeFromRead();
        //if depth change
        if(this->GetDepth())
        {
            lock.unlock();
            return this->Put(std::move(key),std::move(value));
        }
        return this->PutToRoot(std::move(key),std::move(value));
    }
    //find path
    auto path{this->GetPath(key)};
    sharpen::Optional<sharpen::BtBlock> nextBlock;
    //lock leaf(X)
    this->LockBlockForWrite(*path.back());
    //if we need to split or block may overflow
    if (path.back()->GetSize() == this->maxRecordsOfBlock_ || path.back()->GetSize() < path.back()->GetUsedSize() + key.GetSize() + value.GetSize() + 20)
    {
        //unlock leaf(X)
        this->UnlockBlock(*path.back());
        //lock root lock(S -> X)
        this->GetRootLock().UpgradeFromRead();
        if(!this->GetDepth())
        {
            return this->PutToRoot(std::move(key),std::move(value));
        }
        //refind path
        path = this->GetPath(key);
        //we need to change upper node
        //motify leaf
        sharpen::Optional<sharpen::BtBlock> overBlock;
        sharpen::FilePointer oldPointer;
        auto leaf{path.back()};
        oldPointer.offset_ = leaf->GetSwitzzPointer();
        oldPointer.size_ = leaf->GetBlockSize();
        leaf->Put(std::move(key),std::move(value));
        //if > max count
        //split the block
        if(leaf->GetSize() > this->maxRecordsOfBlock_)
        {
            overBlock.Construct(leaf->Split());
            auto &over{overBlock.Get()};
            over.Next() = leaf->Next();
            over.Prev() = oldPointer;
            this->AllocAndWriteBlock(over);
            leaf->Next().offset_ = over.GetSwitzzPointer();
            leaf->Next().size_ = over.GetBlockSize();
        }
        this->AllocAndWriteBlock(*leaf);
        //if not need to change upper
        if(!overBlock.Exist() && oldPointer.offset_ == leaf->GetSwitzzPointer())
        {
            return;
        }
        path.pop_back();
        for (auto begin = path.rbegin(),end = path.rend(); begin != end; ++begin)
        {
            //if not need to change upper
            if(oldPointer.offset_ == leaf->GetSwitzzPointer() && !overBlock.Exist())
            {
                return;
            }
            //motify upper
            auto upper{*begin};
            if(oldPointer.offset_ != leaf->GetSwitzzPointer())
            {
                sharpen::FilePointer p;
                p.offset_ = leaf->GetSwitzzPointer();
                p.size_ = leaf->GetBlockSize();
                sharpen::ByteBuffer buf{sizeof(p)};
                std::memcpy(buf.Data(),&p,sizeof(p));
                upper->Put(leaf->Begin()->GetKey(),std::move(buf));
            }
            if(overBlock.Exist())
            {
                auto &over = overBlock.Get();
                sharpen::FilePointer p;
                p.offset_ = over.GetSwitzzPointer();
                p.size_ = over.GetBlockSize();
                sharpen::ByteBuffer buf{sizeof(p)};
                std::memcpy(buf.Data(),&p,sizeof(p));
                upper->Put(over.Begin()->GetKey(),std::move(buf));
            }
            oldPointer.offset_ = upper->GetSwitzzPointer();
            oldPointer.size_ = upper->GetBlockSize();
            overBlock.Reset();
            leaf = std::move(upper);
            if(leaf->GetSize() > this->maxRecordsOfBlock_)
            {
                overBlock.Construct(leaf->Split());
                auto &over{overBlock.Get()};
                over.Next() = leaf->Next();
                over.Prev() = oldPointer;
                this->AllocAndWriteBlock(over);
                leaf->Next().offset_ = over.GetSwitzzPointer();
                leaf->Next().size_ = over.GetBlockSize();
            }
            this->AllocAndWriteBlock(*leaf);
        }
        if(overBlock.Exist())
        {
            auto &over{overBlock.Get()};
            sharpen::FilePointer p;
            p.offset_ = over.GetSwitzzPointer();
            p.size_ = over.GetBlockSize();
            sharpen::ByteBuffer buf{sizeof(p)};
            std::memcpy(buf.Data(),&p,sizeof(p));
            return this->PutToRoot(over.Begin()->GetKey(),std::move(buf));
        }
        sharpen::FilePointer p;
        p.offset_ = leaf->GetSwitzzPointer();
        p.size_ = leaf->GetBlockSize();
        sharpen::ByteBuffer buf{sizeof(p)};
        std::memcpy(buf.Data(),&p,sizeof(p));
        this->root_.Put(leaf->Begin()->GetKey(),std::move(buf));
        return this->AllocAndWriteBlock(this->root_);
    }
    //motify leaf
    std::unique_lock<sharpen::AsyncReadWriteLock> blockLock{this->GetBlockLock(*path.back()),std::adopt_lock};
    path.back()->Put(std::move(key),std::move(value));
    this->WriteBlock(*path.back());
}

sharpen::Optional<sharpen::ByteBuffer> sharpen::BalancedTable::TryGet(const sharpen::ByteBuffer &key) const
{
    this->GetRootLock().LockRead();
    std::unique_lock<sharpen::AsyncReadWriteLock> lock{this->GetRootLock(),std::adopt_lock};
    if(!this->GetDepth())
    {
        auto ite = this->root_.Find(key);
        if(ite != this->root_.End() && this->CompKey(ite->GetKey(),key) == 0)
        {
            return ite->Value();
        }
        return sharpen::EmptyOpt;
    }
    auto block{this->FindBlock(key)};
    this->LockBlockForRead(*block);
    std::unique_lock<sharpen::AsyncReadWriteLock> blockLock{this->GetBlockLock(*block),std::adopt_lock};
    auto ite = block->Find(key);
    if(ite != block->End() && this->CompKey(ite->GetKey(),key) == 0)
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
    this->GetRootLock().LockRead();
    std::unique_lock<sharpen::AsyncReadWriteLock> lock{this->GetRootLock(),std::adopt_lock};
    if(!this->GetDepth())
    {
        auto ite = this->root_.Find(key);
        if(ite != this->root_.End() && this->CompKey(ite->GetKey(),key) == 0)
        {
            return sharpen::ExistStatus::Exist;
        }
        return sharpen::ExistStatus::NotExist;
    }
    auto block{this->FindBlock(key)};
    this->LockBlockForRead(*block);
    std::unique_lock<sharpen::AsyncReadWriteLock> blockLock{this->GetBlockLock(*block),std::adopt_lock};
    auto ite = block->Find(key);
    if(ite != block->End() && this->CompKey(ite->GetKey(),key) == 0)
    {
        return sharpen::ExistStatus::Exist;
    }
    return sharpen::ExistStatus::NotExist;
}

void sharpen::BalancedTable::DeleteFromRoot(const sharpen::ByteBuffer &key)
{
    this->root_.Delete(key);
    this->WriteBlock(this->root_);
}

void sharpen::BalancedTable::Delete(const sharpen::ByteBuffer &key)
{
    //lock root(S)
    this->GetRootLock().LockRead();
    std::unique_lock<sharpen::AsyncReadWriteLock> lock{this->GetRootLock(),std::adopt_lock};
    if(!this->GetDepth())
    {
        //lock root(S -> I)
        this->GetRootLock().UpgradeFromRead();
        if(this->GetDepth())
        {
            lock.unlock();
            return this->Delete(key);
        }
        return this->DeleteFromRoot(key);
    }
    auto path{this->GetPath(key)};
    //lock leaf(X)
    this->LockBlockForWrite(*path.back());
    if(path.back()->GetSize() == 1)
    {
        //unlock leaf
        this->UnlockBlock(*path.back());
        //lock root(S -> X)
        this->GetRootLock().UpgradeFromRead();
        if(!this->GetDepth())
        {
            return this->DeleteFromRoot(key);
        }
        path = this->GetPath(key);
        auto leaf{path.back()};
        auto ite = leaf->Find(key);
        if(ite != leaf->End() && this->CompKey(ite->GetKey(),key) == 0)
        {
            leaf->Delete(key);
            this->WriteBlock(*leaf);
        }
        //if not need to change upper
        if(!leaf->Empty())
        {
            return;
        }
        path.pop_back();
        for (auto begin = path.rbegin(),end = path.rend(); begin != end; ++begin)
        {
            if (!leaf || !leaf->Empty())
            {
                return;
            }
            auto upper{*begin};
            upper->FuzzingDelete(key);
            this->WriteBlock(*upper);
            sharpen::FilePointer pointer;
            pointer.size_ = leaf->GetBlockSize();
            pointer.offset_ = leaf->GetSwitzzPointer();
            this->DeleteFromCache(pointer);
            this->FreeMemory(pointer);
            leaf = std::move(upper);
        }
        this->root_.FuzzingDelete(key);
        if(this->root_.GetSize() == 1)
        {
            auto p{this->root_.Begin()->Value().As<sharpen::FilePointer>()};
            auto newRoot{this->LoadBlock(p)};
            std::memset(&newRoot.Prev(),0,sizeof(newRoot.Prev()));
            std::memset(&newRoot.Next(),0,sizeof(newRoot.Next()));
            this->DeleteFromCache(p);
            this->WriteRootPointer(p);
            this->root_ = std::move(newRoot);
            return;
        }
        this->WriteBlock(this->root_);
        return;
    }
    std::unique_lock<sharpen::AsyncReadWriteLock> blockLock{this->GetBlockLock(*path.back()),std::adopt_lock};
    auto leaf{path.back()};
    auto ite = leaf->Find(key);
    if(ite != leaf->End() && this->CompKey(ite->GetKey(),key) == 0)
    {
        leaf->Delete(key);
        this->WriteBlock(*leaf);
    }
}

std::shared_ptr<const sharpen::BtBlock> sharpen::BalancedTable::FindBlock(const sharpen::ByteBuffer &key,bool doCache) const
{
    sharpen::Size depth{this->GetDepth()};
    if(depth)
    {
        auto ite = this->root_.FuzzingFind(key);
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
            ite = block->FuzzingFind(key);
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