#include <sharpen/LevelTableScanner.hpp>

#include <sharpen/LevelTable.hpp>

sharpen::Optional<sharpen::ByteBuffer> sharpen::LevelTableScanner::SelectKeyFromMemTable(const MemTable &table,const sharpen::ByteBuffer *before,const sharpen::ByteBuffer *after) const
{
    table.GetLock().LockRead();
    std::unique_lock<sharpen::AsyncReadWriteLock> lock{table.GetLock(),std::adopt_lock};
    if(!table.Empty())
    {
        auto begin = table.Begin(),end = table.End();
        if(after)
        {
            begin = table.BinaryFind(*after);
        }
        for (; begin != end; ++begin)
        {
            if(before)
            {
                std::int32_t r{this->table_->CompareKeys(begin->first,*before)};
                if(r == 1)
                {
                    return sharpen::EmptyOpt;
                }
            }
            if(!begin->second.IsDeleted())
            {
                if(after)
                {
                    std::int32_t r{this->table_->CompareKeys(begin->first,*after)};
                    if(r == 1)
                    {
                        break;
                    }
                }
                else
                {
                    break;
                }
            }
        }
        if(begin != end)
        {
            return begin->first;
        }
    }
    return sharpen::EmptyOpt;
}

sharpen::Optional<sharpen::ByteBuffer> sharpen::LevelTableScanner::SelectKeyFromTable(const sharpen::SortedStringTable &table,const sharpen::ByteBuffer *before,const sharpen::ByteBuffer *after) const
{
    if(!table.Root().IndexBlock().Empty())
    {
        const auto &indexBlock{table.Root().IndexBlock()};
        auto begin = indexBlock.Begin(),end = indexBlock.End();
        if(after)
        {
            begin = indexBlock.Find(*after);
        }
        for (; begin != end; ++begin)
        {
            auto block{table.FindBlock(begin->GetKey())};
            auto keyBegin = block->TwoWayBegin(),keyEnd = block->TwoWayEnd();
            for (; keyBegin != keyEnd; ++keyBegin)
            {
                if(before)
                {
                    std::int32_t r{this->table_->CompareKeys(keyBegin->GetKey(),*before)};
                    if(r == 1)
                    {
                        return sharpen::EmptyOpt;
                    }
                }
                if(!keyBegin->Value().Empty())
                {
                    if(after)
                    {
                        std::int32_t r{this->table_->CompareKeys(keyBegin->GetKey(),*after)};
                        if(r == 1)
                        {
                            break;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }
            if(keyBegin != keyEnd)
            {
                return keyBegin->GetKey();
            }
        }
    }
    return sharpen::EmptyOpt;
}

//(after,before]
sharpen::Optional<sharpen::ByteBuffer> sharpen::LevelTableScanner::SelectKey(const sharpen::ByteBuffer *before,const sharpen::ByteBuffer *after) const
{
    const auto &memTable{this->table_->GetMemoryTable()};
    sharpen::Optional<sharpen::ByteBuffer> selectedKey{this->SelectKeyFromMemTable(memTable,before,after)};
    for (auto begin = this->immTables_.begin(),end = this->immTables_.end(); begin != end; ++begin)
    {
        const auto immTable{*begin};
        sharpen::Optional<sharpen::ByteBuffer> tmp{this->SelectKeyFromMemTable(*immTable,selectedKey.Exist() ? &selectedKey.Get():before,after)};
        if(tmp.Exist())
        {
            selectedKey = std::move(tmp);
        }
    }
    for (auto begin = this->tables_.begin(),end = this->tables_.end(); begin != end; ++begin)
    {
        if(selectedKey.Exist() && this->table_->CompareKeys(selectedKey.Get(),begin->BeginKey()) == -1)
        {
            break;
        }
        if(this->useCache_)
        {
            auto table{this->table_->GetTable(begin->GetId())};
            sharpen::Optional<sharpen::ByteBuffer> tmp{this->SelectKeyFromTable(*table,selectedKey.Exist()?&selectedKey.Get():before,after)};
            if(tmp.Exist())
            {
                selectedKey = std::move(tmp);
            }
        }
        else
        {
            auto table{this->table_->GetTableCopy(begin->GetId())};
            sharpen::Optional<sharpen::ByteBuffer> tmp{this->SelectKeyFromTable(table,selectedKey.Exist()?&selectedKey.Get():before,after)};
            if(tmp.Exist())
            {
                selectedKey = std::move(tmp);
            }
        }
    }
    return selectedKey;
}

sharpen::Optional<sharpen::ByteBuffer> sharpen::LevelTableScanner::SelectNextKey() const
{
    if(this->isEmpty_)
    {
        return sharpen::EmptyOpt;
    }
    if(this->IsRangeQuery())
    {
        return this->SelectKey(&this->GetRangeEnd(),&this->currentKey_);
    }
    return this->SelectKey(nullptr,&this->currentKey_);
}

sharpen::LevelTableScanner::LevelTableScanner(const sharpen::LevelTable &table)
    :table_(&table)
    ,levelLock_()
    ,tables_()
    ,range_(sharpen::EmptyOpt)
    ,currentKey_()
    ,useCache_(true)
    ,isEmpty_(false)
{
    this->table_->GetLevelLock().LockRead();
    std::unique_lock<sharpen::AsyncReadWriteLock> lock{this->table_->GetLevelLock(),std::adopt_lock};
    this->table_->TableScan(std::back_inserter(this->tables_));
    auto key{this->SelectKey(nullptr,nullptr)};
    if(key.Exist())
    {
        this->currentKey_ = std::move(key.Get());
        this->levelLock_ = std::move(lock);
    }
    else
    {
        this->isEmpty_ = true;
    }
}

sharpen::LevelTableScanner::LevelTableScanner(const sharpen::LevelTable &table,const sharpen::ByteBuffer &beginKey,const sharpen::ByteBuffer &endKey)
    :table_(&table)
    ,levelLock_()
    ,tables_()
    ,range_(beginKey,endKey)
    ,currentKey_()
    ,useCache_(true)
    ,isEmpty_(false)
{
    this->table_->GetLevelLock().LockRead();
    std::unique_lock<sharpen::AsyncReadWriteLock> lock{this->table_->GetLevelLock(),std::adopt_lock};
    this->table_->TableScan(std::back_inserter(this->tables_));
    auto key{this->SelectKey(&this->GetRangeEnd(),&this->GetRangeBegin())};
    if(key.Exist())
    {
        this->currentKey_ = std::move(key.Get());
        this->levelLock_ = std::move(lock);
    }
    else
    {
        this->isEmpty_ = true;
    }
}

bool sharpen::LevelTableScanner::Next()
{
    if(this->isEmpty_)
    {
        return false;
    }
    auto next{this->SelectNextKey()};
    if(next.Exist())
    {
        this->currentKey_ = std::move(next.Get());
        return true;
    }
    return false;
}

bool sharpen::LevelTableScanner::HasNext() const
{
    if(this->isEmpty_)
    {
        return false;
    }
    auto next{this->SelectNextKey()};
    return next.Exist();
}

bool sharpen::LevelTableScanner::Empty() const noexcept
{
    return this->isEmpty_;
}

bool sharpen::LevelTableScanner::Seek(const sharpen::ByteBuffer &key)
{
    if(this->IsRangeQuery())
    {
        std::int32_t r{this->table_->CompareKeys(key,this->GetRangeBegin())};
        if (r == -1)
        {
            return false;
        }
        r = this->table_->CompareKeys(key,this->GetRangeEnd());
        if(r == 1)
        {
            return false;
        }
    }
    if(this->table_->Exist(key) == sharpen::ExistStatus::Exist)
    {
        this->currentKey_ = key;
        return true;
    }
    sharpen::Optional<sharpen::ByteBuffer> opt;
    if(this->IsRangeQuery())
    {
        opt = this->SelectKey(&this->GetRangeEnd(),&key);
    }
    else
    {
        opt = this->SelectKey(nullptr,&key);
    }
    if(opt.Exist())
    {
        this->currentKey_ = std::move(opt.Get());
    }
    return opt.Exist();
}

sharpen::ByteBuffer sharpen::LevelTableScanner::GetCurrentValue() const
{
    return this->table_->Get(this->currentKey_);
}