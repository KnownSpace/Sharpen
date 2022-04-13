#include <sharpen/BalancedTableScanner.hpp>

#include <iterator>

#include <sharpen/IteratorOps.hpp>
#include <sharpen/BalancedTable.hpp>

sharpen::BalancedTableScanner::BalancedTableScanner(const sharpen::BalancedTable &table)
    :table_(&table)
    ,rootLock_()
    ,pointers_()
    ,range_()
    ,currentKey_()
    ,currentPointer_(0)
    ,useCache_(true)
{
    auto &rootLock{this->table_->GetRootLock()};
    rootLock.LockRead();
    std::unique_lock<sharpen::AsyncReadWriteLock> lock{rootLock,std::adopt_lock};
    this->table_->TableScan(std::back_inserter(this->pointers_));
    if(!this->pointers_.empty())
    {
        auto first = this->pointers_.front();
        this->table_->GetBlockLock(first.offset_).LockRead();
        std::unique_lock<sharpen::AsyncReadWriteLock> blockLock{this->table_->GetBlockLock(first.offset_),std::adopt_lock};
        auto block{this->table_->LoadBlockCache(first)};
        this->currentKey_ = block->Begin()->GetKey();
    }
    this->rootLock_ = std::move(lock);
}

sharpen::BalancedTableScanner::BalancedTableScanner(const sharpen::BalancedTable &table,const sharpen::ByteBuffer &beginKey,const sharpen::ByteBuffer &endKey)
    :table_(&table)
    ,rootLock_()
    ,pointers_()
    ,range_()
    ,currentKey_()
    ,currentPointer_(0)
    ,useCache_(true)
{
    auto &rootLock{this->table_->GetRootLock()};
    rootLock.LockRead();
    std::unique_lock<sharpen::AsyncReadWriteLock> lock{rootLock,std::adopt_lock};
    this->table_->TableScan(std::back_inserter(this->pointers_));
    if(!this->pointers_.empty())
    {
        this->range_.Construct(beginKey,endKey);
        auto first = this->pointers_.front();
        this->table_->GetBlockLock(first.offset_).LockRead();
        std::unique_lock<sharpen::AsyncReadWriteLock> blockLock{this->table_->GetBlockLock(first.offset_),std::adopt_lock};
        auto block{this->table_->LoadBlockCache(first)};
        if(block->Exist(beginKey) == sharpen::ExistStatus::Exist)
        {
            this->currentKey_ = beginKey;
        }
        else
        {
            this->currentKey_ = block->Begin()->GetKey();
        }
    }
    this->rootLock_ = std::move(lock);
}

sharpen::ByteBuffer sharpen::BalancedTableScanner::GetCurrentValue() const
{
    if(this->pointers_.empty())
    {
        return sharpen::ByteBuffer{};
    }
    sharpen::FilePointer currentPointer{this->pointers_[this->currentPointer_]};
    if(this->useCache_)
    {
        auto block{this->table_->LoadBlockCache(currentPointer)};
        return block->Get(this->currentKey_);
    }
    auto block{this->table_->LoadBlock(currentPointer)};
    return std::move(*block.Begin()).MoveKey();
}

bool sharpen::BalancedTableScanner::HasNext(const sharpen::BtBlock &block,const sharpen::ByteBuffer &key) const
{
    if(this->IsRangeQuery())
    {
        if(block.Exist(this->GetRangeEnd()) != sharpen::ExistStatus::NotExist)
        {
            return this->table_->CompareKeys(key,this->GetRangeEnd()) != 0;
        }
    }
    return this->table_->CompareKeys(block.ReverseBegin()->GetKey(),key) != 0;
}

bool sharpen::BalancedTableScanner::HasNext() const
{
    if(this->IsEmpty())
    {
        return false;
    }
    if(this->currentPointer_ != this->pointers_.size() - 1)
    {
        return true;
    }
    sharpen::FilePointer currentPointer{this->pointers_[this->currentPointer_]};
    auto &blockLock{this->table_->GetBlockLock(currentPointer.offset_)};
    blockLock.LockRead();
    std::unique_lock<sharpen::AsyncReadWriteLock> lock{blockLock,std::adopt_lock};
    if(this->useCache_)
    {
        auto block{this->table_->LoadBlockCache(currentPointer)};
        return this->HasNext(*block,this->currentKey_);
    }
    auto block{this->table_->LoadBlock(currentPointer)};
    return this->HasNext(block,this->currentKey_);
}

bool sharpen::BalancedTableScanner::Next()
{
    if(this->IsEmpty())
    {
        return false;
    }
    sharpen::FilePointer currentPointer{this->pointers_[this->currentPointer_]};
    auto &blockLock{this->table_->GetBlockLock(currentPointer.offset_)};
    blockLock.LockRead();
    std::unique_lock<sharpen::AsyncReadWriteLock> lock{blockLock,std::adopt_lock};
    if(this->currentPointer_ != this->pointers_.size() - 1)
    {
        if (this->useCache_)
        {
            auto block{this->table_->LoadBlockCache(currentPointer)};
            auto ite = sharpen::IteratorForward(block->Find(this->currentKey_),1);
            if(ite == block->End())
            {
                ++this->currentPointer_;
                currentPointer = this->pointers_[this->currentPointer_];
                block = this->table_->LoadBlockCache(currentPointer);
                this->currentKey_ = block->Begin()->GetKey();
            }
            else
            {
                this->currentKey_ = ite->GetKey();
            }
        }
        else
        {
            auto block{this->table_->LoadBlock(currentPointer)};
            auto ite = sharpen::IteratorForward(block.Find(this->currentKey_),1);
            if(ite == block.End())
            {
                ++this->currentPointer_;
                currentPointer = this->pointers_[this->currentPointer_];
                block = this->table_->LoadBlock(currentPointer);
                this->currentKey_ = std::move(*block.Begin()).MoveKey();
            }
            else
            {
                this->currentKey_ = std::move(*ite).MoveKey();
            }
        }
        return true;
    }
    if(this->useCache_)
    {
        auto block{this->table_->LoadBlockCache(currentPointer)};
        if(this->IsRangeQuery())
        {
            if(block->Exist(this->GetRangeEnd()) != sharpen::ExistStatus::NotExist)
            {
                if(this->table_->CompareKeys(this->currentKey_,this->GetRangeEnd()) == 0)
                {
                    return false;
                }
            }
        }
        auto ite = sharpen::IteratorForward(block->Find(this->currentKey_),1);
        if(ite != block->End())
        {
            this->currentKey_ = ite->GetKey();
            return true;
        }
        return false;
    }
    auto block{this->table_->LoadBlock(currentPointer)};
    if(this->IsRangeQuery())
    {
        if(block.Exist(this->GetRangeEnd()) != sharpen::ExistStatus::NotExist)
        {
            if(this->table_->CompareKeys(this->currentKey_,this->GetRangeEnd()) == 0)
            {
                return false;
            }
        }
    }
    auto ite = sharpen::IteratorForward(block.Find(this->currentKey_),1);
    if(ite != block.End())
    {
        this->currentKey_ = ite->GetKey();
        return true;
    }
    return false;
}

bool sharpen::BalancedTableScanner::Seek(const sharpen::ByteBuffer &key)
{
    if (this->IsRangeQuery())
    {
        sharpen::Int32 r{this->table_->CompareKeys(key,this->GetRangeBegin())};
        if(r == -1)
        {
            return false;
        }
        r = this->table_->CompareKeys(key,this->GetRangeEnd());
        if(r == 1)
        {
            return false;
        }
    }
    sharpen::Size curr{0};
    while (curr != this->pointers_.size())
    {
        sharpen::FilePointer currentPointer{this->pointers_[curr]};
        auto &blockLock{this->table_->GetBlockLock(currentPointer.offset_)};
        blockLock.LockRead();
        std::unique_lock<sharpen::AsyncReadWriteLock> lock{blockLock,std::adopt_lock};
        if (this->useCache_)
        {
            auto block{this->table_->LoadBlockCache(currentPointer)};
            if(block->Exist(key) == sharpen::ExistStatus::Exist)
            {
                break;
            }
        }
        else
        {
            auto block{this->table_->LoadBlock(this->pointers_[curr])};
            if(block.Exist(key) == sharpen::ExistStatus::Exist)
            {
                break;
            }
        }
    }
    if(curr != this->pointers_.size())
    {
        this->currentKey_ = key;
        this->currentPointer_ = curr;
    }
    return curr != this->pointers_.size();
}