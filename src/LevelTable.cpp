#include <sharpen/LevelTable.hpp>

#include <sharpen/Converter.hpp>
#include <sharpen/FileOps.hpp>

sharpen::ByteBuffer sharpen::LevelTable::currentTableIdKey_;
sharpen::ByteBuffer sharpen::LevelTable::currentViewIdKey_;
sharpen::ByteBuffer sharpen::LevelTable::currentMemTableIdKey_;
sharpen::ByteBuffer sharpen::LevelTable::maxLevelKey_;
sharpen::ByteBuffer sharpen::LevelTable::prevTableIdKey_;
sharpen::ByteBuffer sharpen::LevelTable::prevViewIdKey_;
sharpen::ByteBuffer sharpen::LevelTable::prevMemTableIdKey_;
std::once_flag sharpen::LevelTable::keyFlag_;

std::int32_t sharpen::LevelTable::CompareKeys(const sharpen::ByteBuffer &left,const sharpen::ByteBuffer &right) const noexcept
{
    if(this->comp_)
    {
        return this->comp_(left,right);
    }
    return left.CompareWith(right);
}

sharpen::ByteBuffer sharpen::LevelTable::GetViewKey(std::uint64_t id)
{
    sharpen::ByteBuffer buf{1 + sizeof(id)};
    buf[0] = 'v';
    std::memcpy(buf.Data() + 1,&id,sizeof(id));
    return buf;
}

sharpen::ByteBuffer sharpen::LevelTable::GetComponetKey(std::uint64_t level)
{
    sharpen::ByteBuffer buf{1 + sizeof(level)};
    buf[0] = 'l';
    std::memcpy(buf.Data() + 1,&level,sizeof(level));
    return buf;
}

std::uint64_t sharpen::LevelTable::GetCurrentTableId() const noexcept
{
    return this->manifest_->Get(this->currentTableIdKey_).As<std::uint64_t>();
}

std::uint64_t sharpen::LevelTable::GetCurrentViewId() const noexcept
{
    return this->manifest_->Get(this->currentViewIdKey_).As<std::uint64_t>();
}

std::uint64_t sharpen::LevelTable::GetCurrentMemoryTableId() const noexcept
{
    return this->manifest_->Get(this->currentMemTableIdKey_).As<std::uint64_t>();
}

std::uint64_t sharpen::LevelTable::GetMaxLevel() const noexcept
{
    return this->manifest_->Get(this->maxLevelKey_).As<std::uint64_t>();
}

std::uint64_t sharpen::LevelTable::GetPrevTableId() const noexcept
{
    return this->manifest_->Get(this->prevTableIdKey_).As<std::uint64_t>();
}

std::uint64_t sharpen::LevelTable::GetPrevViewId() const noexcept
{
    return this->manifest_->Get(this->prevViewIdKey_).As<std::uint64_t>();
}

std::uint64_t sharpen::LevelTable::GetPrevMemoryTableId() const noexcept
{
    return this->manifest_->Get(this->prevMemTableIdKey_).As<std::uint64_t>();
}

void sharpen::LevelTable::SetCurrentTableId(std::uint64_t id)
{
    sharpen::ByteBuffer buf{sizeof(id)};
    buf.As<std::uint64_t>() = id;
    this->manifest_->Put(this->currentTableIdKey_,std::move(buf));
}

void sharpen::LevelTable::SetCurrentViewId(std::uint64_t id)
{
    sharpen::ByteBuffer buf{sizeof(id)};
    buf.As<std::uint64_t>() = id;
    this->manifest_->Put(this->currentViewIdKey_,std::move(buf));
}

void sharpen::LevelTable::SetCurrentMemoryTableId(std::uint64_t id)
{
    sharpen::ByteBuffer buf{sizeof(id)};
    buf.As<std::uint64_t>() = id;
    this->manifest_->Put(this->currentMemTableIdKey_,std::move(buf));
}

void sharpen::LevelTable::SetMaxLevel(std::uint64_t level)
{
    sharpen::ByteBuffer buf{sizeof(level)};
    buf.As<std::uint64_t>() = level;
    this->manifest_->Put(this->maxLevelKey_,std::move(buf));
}

void sharpen::LevelTable::SetPrevTableId(std::uint64_t id)
{
    sharpen::ByteBuffer buf{sizeof(id)};
    buf.As<std::uint64_t>() = id;
    this->manifest_->Put(this->prevTableIdKey_,std::move(buf));
}

void sharpen::LevelTable::SetPrevViewId(std::uint64_t id)
{
    sharpen::ByteBuffer buf{sizeof(id)};
    buf.As<std::uint64_t>() = id;
    this->manifest_->Put(this->prevViewIdKey_,std::move(buf));
}

void sharpen::LevelTable::SetPrevMemoryTableId(std::uint64_t id)
{
    sharpen::ByteBuffer buf{sizeof(id)};
    buf.As<std::uint64_t>() = id;
    this->manifest_->Put(this->prevMemTableIdKey_,std::move(buf));
}

void sharpen::LevelTable::InitManifest()
{
    std::string name{this->FormatManifestName()};
    sharpen::FileChannelPtr channel{this->OpenChannel(name.data(),sharpen::FileAccessModel::All,sharpen::FileOpenModel::CreateOrOpen)};
    std::unique_ptr<MemTable> manifest{new MemTable{MemTable::directEraseTag,std::move(channel)}};
    if(!manifest)
    {
        throw std::bad_alloc();
    }
    this->manifest_ = std::move(manifest);
    this->manifest_->Restore();
    if(!this->manifest_->Empty())
    {
        return;
    }
    sharpen::ByteBuffer buf{sizeof(std::uint64_t)};
    buf.As<std::uint64_t>() = 0;
    this->manifest_->Put(this->currentTableIdKey_,buf);
    this->manifest_->Put(this->currentViewIdKey_,buf);
    this->manifest_->Put(this->currentMemTableIdKey_,buf);
    this->manifest_->Put(this->maxLevelKey_,buf);
    this->manifest_->Put(this->prevMemTableIdKey_,buf);
    this->manifest_->Put(this->prevTableIdKey_,buf);
    this->manifest_->Put(this->prevViewIdKey_,std::move(buf));
}

void sharpen::LevelTable::InitManifestKeys()
{
    char currentTableIdKey[] = "ctid";
    char currentViewIdKey[] = "cvid";
    char maxLevelKey[] = "ml";
    char currentMemIdKey[] = "cmid";
    char prevTableIdKey[] = "ptid";
    char prevViewIdKey[] = "pvid";
    char prevMemIdKey[] = "pmid";
    {
        sharpen::ByteBuffer buf{currentTableIdKey,sizeof(currentTableIdKey) - 1};
        Self::currentTableIdKey_ = std::move(buf);
    }
    {
        sharpen::ByteBuffer buf{currentViewIdKey,sizeof(currentViewIdKey) - 1};
        Self::currentViewIdKey_ = std::move(buf);
    }
    {
        sharpen::ByteBuffer buf{maxLevelKey,sizeof(maxLevelKey) - 1};
        Self::maxLevelKey_ = std::move(buf);
    }
    {
        sharpen::ByteBuffer buf{currentMemIdKey,sizeof(currentMemIdKey) - 1};
        Self::currentMemTableIdKey_ = std::move(buf);
    }
    {
        sharpen::ByteBuffer buf{prevTableIdKey,sizeof(prevTableIdKey) - 1};
        Self::prevTableIdKey_ = std::move(buf);
    }
    {
        sharpen::ByteBuffer buf{prevViewIdKey,sizeof(prevViewIdKey) - 1};
        Self::prevViewIdKey_ = std::move(buf);
    }
    {
        sharpen::ByteBuffer buf{prevMemIdKey,sizeof(prevMemIdKey) - 1};
        Self::prevMemTableIdKey_ = std::move(buf);
    }
}

sharpen::FileChannelPtr sharpen::LevelTable::OpenChannel(const char *name,sharpen::FileAccessModel accessModel,sharpen::FileOpenModel openModel) const
{
    assert(this->engine_ != nullptr);
    sharpen::FileChannelPtr channel{nullptr};
    if(this->fileGenerator_)
    {
        channel = this->fileGenerator_(name,accessModel,openModel);
    }
    else
    {
        channel = sharpen::MakeFileChannel(name,accessModel,openModel);
    }
    channel->Register(*this->engine_);
    return channel;
}

sharpen::LevelView &sharpen::LevelTable::GetView(std::uint64_t id)
{
    this->viewLock_->LockRead();
    std::unique_lock<sharpen::AsyncReadWriteLock> lock{*this->viewLock_,std::adopt_lock};
    auto ite = this->viewMap_.find(id);
    if(ite != this->viewMap_.end())
    {
        return ite->second;
    }
    this->viewLock_->UpgradeFromRead();
    sharpen::LevelView view{id};
    view.SetComparator(this->comp_);
    sharpen::ByteBuffer key{this->GetViewKey(id)};
    if(this->manifest_->Exist(key) == sharpen::ExistStatus::Exist)
    {
        view.LoadFrom(this->manifest_->Get(key));
    }
    this->viewMap_.emplace(id,std::move(view));
    return this->viewMap_[id];
}

const sharpen::LevelView &sharpen::LevelTable::GetView(std::uint64_t id) const
{
    this->viewLock_->LockRead();
    std::unique_lock<sharpen::AsyncReadWriteLock> lock{*this->viewLock_,std::adopt_lock};
    auto ite = this->viewMap_.find(id);
    if(ite != this->viewMap_.end())
    {
        return ite->second;
    }
    this->viewLock_->UpgradeFromRead();
    sharpen::LevelView view{id};
    view.SetComparator(this->comp_);
    sharpen::ByteBuffer key{this->GetViewKey(id)};
    if(this->manifest_->Exist(key) == sharpen::ExistStatus::Exist)
    {
        view.LoadFrom(this->manifest_->Get(key));
    }
    this->viewMap_.emplace(id,std::move(view));
    return this->viewMap_[id];
}

void sharpen::LevelTable::SaveView(std::uint64_t id,const sharpen::LevelView &view)
{
    sharpen::ByteBuffer buf;
    view.StoreTo(buf);
    sharpen::ByteBuffer key{this->GetViewKey(id)};
    this->manifest_->Put(std::move(key),std::move(buf));
}

sharpen::LevelComponent &sharpen::LevelTable::GetComponent(std::uint64_t id)
{
    this->componentLock_->LockRead();
    std::unique_lock<sharpen::AsyncReadWriteLock> lock{*this->componentLock_,std::adopt_lock};
    auto ite = this->componentMap_.find(id);
    if(ite != this->componentMap_.end())
    {
        return ite->second;
    }
    this->componentLock_->UpgradeFromRead();
    sharpen::ByteBuffer key{this->GetComponetKey(id)};
    sharpen::LevelComponent component;
    if(this->manifest_->Exist(key) == sharpen::ExistStatus::Exist)
    {
        component.LoadFrom(this->manifest_->Get(key));
    }
    this->componentMap_.emplace(id,std::move(component));
    return this->componentMap_[id];
}

const sharpen::LevelComponent &sharpen::LevelTable::GetComponent(std::uint64_t id) const
{
    this->componentLock_->LockRead();
    std::unique_lock<sharpen::AsyncReadWriteLock> lock{*this->componentLock_,std::adopt_lock};
    auto ite = this->componentMap_.find(id);
    if(ite != this->componentMap_.end())
    {
        return ite->second;
    }
    this->componentLock_->UpgradeFromRead();
    sharpen::ByteBuffer key{this->GetComponetKey(id)};
    sharpen::LevelComponent component;
    if(this->manifest_->Exist(key) == sharpen::ExistStatus::Exist)
    {
        component.LoadFrom(this->manifest_->Get(key));
    }
    this->componentMap_.emplace(id,std::move(component));
    return this->componentMap_[id];
}

void sharpen::LevelTable::SaveComponent(std::uint64_t id,const sharpen::LevelComponent &component)
{
    sharpen::ByteBuffer buf;
    component.StoreTo(buf);
    sharpen::ByteBuffer key{this->GetComponetKey(id)};
    this->manifest_->Put(std::move(key),std::move(buf));
}

std::size_t sharpen::LevelTable::GetTableCount(const sharpen::LevelComponent &component) const
{
    std::size_t count{0};
    for (auto begin = component.Begin(),end = component.End(); begin != end; ++begin)
    {
        const sharpen::LevelView *view{&this->GetView(*begin)};
        count += view->GetSize();
    }
    return count;
}

std::string sharpen::LevelTable::FormatTableName(std::uint64_t id) const
{
    //{tableName}_{id}.{tableExtName}
    std::string name;
    name.resize(this->tableName_.size() + 1 + 44 + 1 + this->tableExtName_.size() + 1,0);
    //copy table name
    std::memcpy(const_cast<char*>(name.data()),this->tableName_.data(),this->tableName_.size());
    //copy '_'
    name[this->tableName_.size()] = '_';
    //copy id
    std::size_t size{sharpen::Itoa(id,10,const_cast<char*>(name.data()) + this->tableName_.size() + 1)};
    //copy .
    name[this->tableName_.size() + 1 + size] = '.';
    //copy tableExtName
    std::memcpy(const_cast<char*>(name.data())  + this->tableName_.size() + 1 + size + 1,this->tableExtName_.data(),this->tableExtName_.size());
    return name;
}

std::string sharpen::LevelTable::FormatMemoryTableName(std::uint64_t id) const
{
    //{tableName}_{id}.{walExtName}
    std::string name;
    name.resize(this->tableName_.size() + 1 + 44 + 1 + this->tableExtName_.size() + 1,0);
    //copy table name
    std::memcpy(const_cast<char*>(name.data()),this->tableName_.data(),this->tableName_.size());
    //copy '_'
    name[this->tableName_.size()] = '_';
    //copy id
    std::size_t size{sharpen::Itoa(id,10,const_cast<char*>(name.data()) + this->tableName_.size() + 1)};
    //copy .
    name[this->tableName_.size() + 1 + size] = '.';
    //copy walExtName
    std::memcpy(const_cast<char*>(name.data())  + this->tableName_.size() + 1 + size + 1,this->walExtName_.data(),this->walExtName_.size());
    return name;
}

std::string sharpen::LevelTable::FormatManifestName() const
{
    //{tableName}_manifest.{walExtName}
    std::string name;
    name.resize(this->tableName_.size() + 1 + 8 + 1 + this->tableExtName_.size() + 1,0);
    //copy table name
    std::memcpy(const_cast<char*>(name.data()),this->tableName_.data(),this->tableName_.size());
    //copy '_'
    name[this->tableName_.size()] = '_';
    //copy manifest
    std::memcpy(const_cast<char*>(name.data()) + this->tableName_.size() + 1,"manifest",sizeof("manifest") - 1);
    //copy .
    name[this->tableName_.size() + 1 + 8] = '.';
    //copy walExtName
    std::memcpy(const_cast<char*>(name.data())  + this->tableName_.size() + 1 + 8 + 1,this->walExtName_.data(),this->walExtName_.size());
    return name;
}

sharpen::SortedStringTable sharpen::LevelTable::MergeTables(const sharpen::LevelComponent &component,std::uint64_t newTableId,bool eraseDeleted,sharpen::Optional<sharpen::SortedStringTable> appendTable)
{
    std::string name{this->FormatTableName(newTableId)};
    //create file
    sharpen::FileChannelPtr channel{this->OpenChannel(name.data(),sharpen::FileAccessModel::All,sharpen::FileOpenModel::CreateNew)};
    //create table
    sharpen::SstOption opt{this->comp_,this->filterBitsOfElement_,0,0};
    sharpen::SortedStringTable table{std::move(channel),opt};
    std::vector<sharpen::SortedStringTable> mergeTables;
    bool ordered{component.GetSize() == 1};
    for (auto begin = component.Begin(),end = component.End(); begin != end; ++begin)
    {
        sharpen::LevelView *view{&this->GetView(*begin)};
        for (auto tbBegin = view->Begin(),tbEnd = view->End(); tbBegin != tbEnd; ++tbBegin)
        {
            mergeTables.emplace_back(this->LoadTable(tbBegin->GetId()));
        }
    }
    if(appendTable.Exist())
    {
        //get key
        if(ordered)
        {
            sharpen::SstDataBlock firstBlock{appendTable.Get().LoadBlock(appendTable.Get().Root().IndexBlock().Begin()->Block())};
            sharpen::SstDataBlock lastBlock{appendTable.Get().LoadBlock(appendTable.Get().Root().IndexBlock().ReverseBegin()->Block())};
            const sharpen::ByteBuffer &firstKey = firstBlock.Begin()->First().GetKey();
            const sharpen::ByteBuffer &lastKey = lastBlock.ReverseBegin()->Last().GetKey();
            sharpen::LevelView *view{nullptr};
            std::uint64_t viewId{*component.ReverseBegin()};
            view = &this->GetView(viewId);
            if(!view->IsNotOverlapped(firstKey,lastKey))
            {
                ordered = false;
            }
        }
        mergeTables.emplace_back(std::move(appendTable.Get()));
    }
    //merge
    sharpen::SstBuildOption buildOpt{eraseDeleted,this->filterBitsOfElement_,this->blockSize_};
    table.Merge(mergeTables.begin(),mergeTables.end(),buildOpt,ordered);
    for (auto begin = mergeTables.begin(),end = mergeTables.end(); begin != end; ++begin)
    {
        begin->Close();
    }
    return table;
}

void sharpen::LevelTable::AddToComponent(sharpen::SortedStringTable table,std::uint64_t tableId,std::uint64_t componentId)
{
    //load component
    sharpen::LevelComponent *component{&this->GetComponent(componentId)};
    //load level
    std::uint64_t level{this->GetMaxLevel()};
    if(!table.Root().IndexBlock().Empty())
    {
        //update level if we need
        if(level < componentId)
        {
            level = componentId;
            this->SetMaxLevel(level);
        }
        //get key
        sharpen::SstDataBlock firstBlock{table.LoadBlock(table.Root().IndexBlock().Begin()->Block())};
        sharpen::SstDataBlock lastBlock{table.LoadBlock(table.Root().IndexBlock().ReverseBegin()->Block())};
        const sharpen::ByteBuffer &firstKey = firstBlock.Begin()->First().GetKey();
        const sharpen::ByteBuffer &lastKey = lastBlock.ReverseBegin()->Last().GetKey();
        //if empty component
        if(component->Empty())
        {
            std::uint64_t viewId{this->GetCurrentViewId()};
            this->SetCurrentViewId(viewId + 1);
            sharpen::LevelView view{viewId};
            view.Put(firstKey,lastKey,tableId);
            this->SetPrevTableId(this->GetCurrentTableId());
            this->SaveView(viewId,view);
            component->Put(viewId);
            this->SetPrevViewId(viewId + 1);
            this->SaveComponent(componentId,*component);
            return;
        }
        std::size_t tableNum{this->GetTableCount(*component)};
        //if table number > max
        if (this->maxTableOfComponent_ != 0 && tableNum == this->maxTableOfComponent_)
        {
            //copy old views
            std::vector<std::uint64_t> oldViews{component->Begin(),component->End()};
            //merge tables
            std::uint64_t newTableId{this->GetCurrentTableId()};
            this->SetCurrentTableId(newTableId + 1);
            sharpen::SortedStringTable newTable{this->MergeTables(*component,newTableId,componentId == this->GetMaxLevel() && component->GetSize() == 1,std::move(table))};
            //add to upper component
            try
            {
                this->AddToComponent(std::move(newTable),newTableId,componentId + 1);
            }
            catch(const std::exception&)
            {
                this->SetCurrentTableId(newTableId);
                throw;
            }
            //clear component
            component->Clear();
            this->SaveComponent(componentId,*component);
            //clear views
            for (auto begin = oldViews.begin(),end = oldViews.end(); begin != end; ++begin)
            {
                sharpen::LevelView *view = &this->GetView(*begin);
                for (auto tbBegin = view->Begin(),tbEnd = view->End(); tbBegin != tbEnd; ++tbBegin)
                {
                    this->DeleteTable(tbBegin->GetId());
                }
                //delete view
                this->manifest_->Delete(this->GetViewKey(*begin));
            }
            this->DeleteTable(tableId);
            //reset max level
            if(componentId && this->GetMaxLevel() == componentId)
            {
                this->SetMaxLevel(componentId - 1);
            }
            return;
        }
        sharpen::LevelView *view{nullptr};
        std::uint64_t viewId{*component->ReverseBegin()};
        view = &this->GetView(viewId);
        bool r{view->TryPut(firstKey,lastKey,tableId)};
        if(!r)
        {
            //if view number > max 
            if(this->maxViewOfComponent_ != 0 && component->GetSize() == this->maxViewOfComponent_)
            {
                //copy old views
                std::vector<std::uint64_t> oldViews{component->Begin(),component->End()};
                //merge tables
                std::uint64_t newTableId{this->GetCurrentTableId()};
                this->SetCurrentTableId(newTableId + 1);
                sharpen::SortedStringTable newTable{this->MergeTables(*component,newTableId,componentId == this->GetMaxLevel() && component->GetSize() == 1,std::move(table))};
                //add to upper component
                try
                {
                    this->AddToComponent(std::move(newTable),newTableId,componentId + 1);
                }
                catch(const std::exception&)
                {
                    this->SetCurrentTableId(newTableId);
                    throw;
                }
                //clear component
                component->Clear();
                this->SaveComponent(componentId,*component);
                //clear views
                for (auto begin = oldViews.begin(),end = oldViews.end(); begin != end; ++begin)
                {
                    view = &this->GetView(*begin);
                    for (auto tbBegin = view->Begin(),tbEnd = view->End(); tbBegin != tbEnd; ++tbBegin)
                    {
                        this->DeleteTable(tbBegin->GetId());
                    }
                    //delete view
                    this->manifest_->Delete(this->GetViewKey(*begin));
                }
                this->DeleteTable(tableId);
                //reset max level
                if(componentId && this->GetMaxLevel() == componentId)
                {
                    this->SetMaxLevel(componentId - 1);
                }
                return;
            }
            viewId = this->GetCurrentViewId();
            this->SetCurrentViewId(viewId + 1);
            sharpen::LevelView newView{viewId};
            newView.Put(firstKey,lastKey,tableId);
            this->SetPrevTableId(this->GetCurrentTableId());
            this->SaveView(viewId,newView);
            component->Put(viewId);
            this->SaveComponent(componentId,*component);
            this->SetPrevViewId(viewId + 1);
            return;
        }
        this->SetPrevTableId(this->GetCurrentTableId());
        this->SaveView(viewId,*view);
        return;
    }
    this->SetPrevTableId(this->GetCurrentTableId());
    if(level == componentId && component->Empty())
    {
        this->SetMaxLevel(level - 1);
    }
}

sharpen::SortedStringTable sharpen::LevelTable::LoadTable(std::uint64_t id) const
{
    std::string name{this->FormatTableName(id)};
    sharpen::FileChannelPtr channel{this->OpenChannel(name.data(),sharpen::FileAccessModel::Read,sharpen::FileOpenModel::CreateOrOpen)};
    sharpen::SstOption opt{this->comp_,this->filterBitsOfElement_,this->blockCacheSize_,this->blockCacheSize_};
    return {channel,opt};
}

void sharpen::LevelTable::DeleteTableFromCache(std::uint64_t id)
{
    this->tableCaches_.Delete(reinterpret_cast<char*>(&id),reinterpret_cast<char*>(&id) + sizeof(id));
}

void sharpen::LevelTable::DeleteTable(std::uint64_t id)
{
    this->DeleteTableFromCache(id);
    std::string name{this->FormatTableName(id)};
    if(sharpen::ExistFile(name.data()))
    {
        sharpen::RemoveFile(name.data());
    }
}

std::shared_ptr<sharpen::SortedStringTable> sharpen::LevelTable::LoadTableFromCache(std::uint64_t id) const
{
    return this->tableCaches_.Get(reinterpret_cast<char*>(&id),reinterpret_cast<char*>(&id) + sizeof(id));
}

std::shared_ptr<sharpen::SortedStringTable> sharpen::LevelTable::LoadTableCache(std::uint64_t id) const
{
    auto table{this->LoadTableFromCache(id)};
    if(!table)
    {
        table = this->tableCaches_.GetOrEmplace(reinterpret_cast<char*>(&id),reinterpret_cast<char*>(&id) + sizeof(id),this->LoadTable(id));
    }
    return table;
}

std::unique_ptr<sharpen::LevelTable::MemTable> sharpen::LevelTable::MakeNewMemoryTable()
{
    std::uint64_t memId{this->GetCurrentMemoryTableId()};
    std::string name{this->FormatMemoryTableName(memId + 1)};
    this->SetCurrentMemoryTableId(memId + 1);
    sharpen::FileChannelPtr channel = this->OpenChannel(name.data(),sharpen::FileAccessModel::All,sharpen::FileOpenModel::CreateNew);
    std::unique_ptr<MemTable> memTable{new MemTable{sharpen::MemoryTableComparator{this->comp_},std::move(channel)}};
    if(!memTable)
    {
        throw std::bad_alloc();
    }
    return memTable;
}

void sharpen::LevelTable::InitImmutableTables()
{
    std::uint64_t beginKey{this->GetPrevMemoryTableId()};
    std::uint64_t endKey{this->GetCurrentMemoryTableId()};
    this->imMems_.reserve((std::max)(this->maxSizeOfImMems_,endKey - beginKey));
    while (beginKey != endKey)
    {
        std::string name{this->FormatMemoryTableName(beginKey)};
        sharpen::FileChannelPtr channel{this->OpenChannel(name.data(),sharpen::FileAccessModel::All,sharpen::FileOpenModel::CreateOrOpen)};
        std::unique_ptr<MemTable> memTable{new MemTable{sharpen::MemoryTableComparator{this->comp_},std::move(channel)}};
        if(!memTable)
        {
            throw std::bad_alloc();
        }
        memTable->Restore();
        this->imMems_.emplace_back(std::move(memTable));
        ++beginKey;
    }
    if(this->imMems_.size() >= this->maxSizeOfImMems_)
    {
        this->DummpImmutableTables();
    }
}

void sharpen::LevelTable::InitMemoryTable()
{
    std::uint64_t memId{this->GetCurrentMemoryTableId()};
    std::string name{this->FormatMemoryTableName(memId)};
    sharpen::FileChannelPtr channel{this->OpenChannel(name.data(),sharpen::FileAccessModel::All,sharpen::FileOpenModel::CreateOrOpen)};
    std::unique_ptr<MemTable> memTable{new MemTable{sharpen::MemoryTableComparator{this->comp_},std::move(channel)}};
    if(!memTable)
    {
        throw std::bad_alloc();
    }
    this->mem_ = std::move(memTable);
    this->mem_->Restore();
}

void sharpen::LevelTable::GcTables()
{
    std::uint64_t begin{this->GetPrevTableId()};
    std::uint64_t end{this->GetCurrentTableId()};
    if (begin != end)
    {
        std::vector<std::uint64_t> tableIds;
        std::vector<std::uint64_t> viewIds;
        std::size_t levelSize{this->GetMaxLevel() + 1};
        tableIds.reserve(end - begin);
        viewIds.reserve(levelSize);
        //for each every level
        for (std::size_t level = 0; level != levelSize; ++level)
        {
            viewIds.clear();
            //get component
            sharpen::LevelComponent *component{&this->GetComponent(level)};
            //for each every view
            for (auto viewBegin = component->Begin(),viewEnd = component->End(); viewBegin != viewEnd; ++viewBegin)
            {
                tableIds.clear();
                sharpen::LevelView *view{&this->GetView(*viewBegin)};
                //for each every tables
                //but not motify container
                for (auto tableBegin = view->Begin(),tableEnd = view->End(); tableBegin != tableEnd; ++tableBegin)
                {
                    std::uint64_t id{tableBegin->GetId()};
                    if(id > begin && id <= end)
                    {
                        tableIds.emplace_back(id);
                    }
                }
                if(tableIds.size() == view->GetSize())
                {
                    //if we need to delete the view
                    viewIds.emplace_back(*viewBegin);
                    continue;
                }
                //delete tables from view
                for (auto ite = tableIds.begin(),idEnd = tableIds.end();ite != idEnd; ++ite)
                {
                    view->Delete(*ite);   
                }
                //save view
                if(!tableIds.empty())
                {
                    this->SaveView(*viewBegin,*view);
                }
            }
            //delete views from component
            for (auto ite = viewIds.begin(),idEnd = viewIds.end();ite != idEnd; ++ite)
            {
                component->Delete(*ite);
            }
            //save component
            if(!viewIds.empty())
            {
                this->SaveComponent(level,*component);
            }
        }
        //set max level
        std::size_t maxLevel{this->GetMaxLevel()};
        while (maxLevel != 0)
        {
            sharpen::LevelComponent *component{&this->GetComponent(maxLevel)};
            if(!component->Empty())
            {
                break;
            }
            --maxLevel;
        }
        if(maxLevel != this->GetMaxLevel())
        {
            this->SetMaxLevel(maxLevel);
        }
        //delete tables   
        while (begin != end)
        {
            this->DeleteTable(end - 1);
            --end;
        }
        this->SetCurrentTableId(end);
    }
}

void sharpen::LevelTable::GcViews()
{
    std::uint64_t begin{this->GetPrevViewId()};
    std::uint64_t end{this->GetCurrentViewId()};
    if (begin != end)
    {
        while (begin != end)
        {
            this->manifest_->Delete(this->GetViewKey(end - 1));
            --end;
        }
        this->SetCurrentViewId(end);
    }
}

sharpen::LevelTable::LevelTable(sharpen::EventEngine &engine,const std::string &tableName,const std::string &tableExtName,const std::string &walExtName,const sharpen::LevelTableOption &opt)
    :tableName_(tableName)
    ,tableExtName_(tableExtName)
    ,walExtName_(walExtName)
    ,componentMap_()
    ,viewMap_()
    ,tableCaches_(opt.GetTableCacheSize())
    ,fileGenerator_(opt.GetFileGenerator())
    ,mem_(nullptr)
    ,manifest_(nullptr)
    ,imMems_()
    ,levelLock_(nullptr)
    ,viewLock_(nullptr)
    ,componentLock_(nullptr)
    ,comp_(opt.GetComparator())
    ,maxViewOfComponent_(opt.GetMaxViewOfComponent())
    ,maxTableOfComponent_(opt.GetMaxTableOfComponent())
    ,blockCacheSize_(opt.GetBlockCacheSize())
    ,filterBitsOfElement_(opt.GetFilterBitsOfElement())
    ,maxSizeOfMem_(opt.GetMaxSizeOfMemoryTable())
    ,maxSizeOfImMems_(opt.GetMaxCountOfImmutableTable())
    ,blockSize_(opt.GetBlockSize())
    ,usedMemory_(nullptr)
    ,engine_(&engine)
{
    using FnPtr = void(*)();
    std::call_once(Self::keyFlag_,static_cast<FnPtr>(&Self::InitManifestKeys));
    this->usedMemory_.reset(new std::atomic_size_t{0});
    if(!this->usedMemory_)
    {
        throw std::bad_alloc();
    }
    //init locks
    this->levelLock_.reset(new sharpen::AsyncReadWriteLock{});
    this->viewLock_.reset(new sharpen::AsyncReadWriteLock{});
    this->componentLock_.reset(new sharpen::AsyncReadWriteLock{});
    if(!this->levelLock_ || !this->viewLock_ || !this->componentLock_)
    {
        throw std::bad_alloc();
    }
    //init manifest
    this->InitManifest();
    this->InitMemoryTable();
    this->InitImmutableTables();
    //restore memory used
    if(!this->mem_->Empty())
    {
        for (auto begin = this->mem_->Begin(),end = this->mem_->End(); begin != end; ++begin)
        {
            this->usedMemory_->fetch_add(begin->first.GetSize(),std::memory_order::memory_order_relaxed);
            if(!begin->second.IsDeleted())
            {
                this->usedMemory_->fetch_add(begin->second.Value().GetSize(),std::memory_order::memory_order_relaxed);
            }   
        }
    }
    //do gc
    this->GcViews();
    this->GcTables();
}

sharpen::LevelTable::LevelTable(Self &&other) noexcept
    :tableName_(std::move(other.tableName_))
    ,tableExtName_(std::move(other.tableExtName_))
    ,walExtName_(std::move(other.walExtName_))
    ,componentMap_(std::move(other.componentMap_))
    ,viewMap_(std::move(other.viewMap_))
    ,tableCaches_(std::move(other.tableCaches_))
    ,fileGenerator_(other.fileGenerator_)
    ,mem_(std::move(other.mem_))
    ,manifest_(std::move(other.manifest_))
    ,imMems_(std::move(other.imMems_))
    ,levelLock_(std::move(levelLock_))
    ,viewLock_(std::move(other.viewLock_))
    ,componentLock_(std::move(other.componentLock_))
    ,comp_(other.comp_)
    ,maxViewOfComponent_(other.maxViewOfComponent_)
    ,maxTableOfComponent_(other.maxTableOfComponent_)
    ,blockCacheSize_(other.blockCacheSize_)
    ,filterBitsOfElement_(other.filterBitsOfElement_)
    ,maxSizeOfMem_(other.maxSizeOfMem_)
    ,maxSizeOfImMems_(other.maxSizeOfImMems_)
    ,blockSize_(other.blockSize_)
    ,usedMemory_(std::move(other.usedMemory_))
    ,engine_(other.engine_)
{}

sharpen::LevelTable &sharpen::LevelTable::operator=(sharpen::LevelTable &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->tableName_ = std::move(other.tableExtName_);
        this->tableExtName_ = std::move(other.tableExtName_);
        this->walExtName_ = std::move(other.walExtName_);
        this->componentMap_ = std::move(other.componentMap_);
        this->viewMap_ = std::move(other.viewMap_);
        this->tableCaches_ = std::move(other.tableCaches_);
        this->fileGenerator_ = other.fileGenerator_;
        this->mem_ = std::move(other.mem_);
        this->manifest_ = std::move(other.manifest_);
        this->imMems_ = std::move(other.imMems_);
        this->levelLock_ = std::move(other.levelLock_);
        this->viewLock_ = std::move(other.viewLock_);
        this->componentLock_ = std::move(other.componentLock_);
        this->comp_ = other.comp_;
        this->maxViewOfComponent_ = other.maxViewOfComponent_;
        this->maxTableOfComponent_ = other.maxTableOfComponent_;
        this->blockCacheSize_ = other.blockCacheSize_;
        this->filterBitsOfElement_ = other.filterBitsOfElement_;
        this->maxSizeOfMem_ = other.maxSizeOfMem_;
        this->maxSizeOfImMems_ = other.maxSizeOfImMems_;
        this->blockSize_ = other.blockSize_;
        this->usedMemory_ = std::move(other.usedMemory_);
        this->engine_ = other.engine_;
    }
    return *this;
}

void sharpen::LevelTable::DummpImmutableTables()
{
    using Map = typename MemTable::MapType;
    Map map{sharpen::MemoryTableComparator{this->comp_}};
    for (auto begin = this->imMems_.rbegin(),end = this->imMems_.rend(); begin != end; ++begin)
    {
        MemTable &imm = **begin;
        for (auto kb = imm.Begin(),ke = imm.End(); kb != ke; ++kb)
        {
            map.emplace(std::move(kb->first),std::move(kb->second));   
        }
    }
    std::uint64_t tableId{this->GetCurrentTableId()};
    this->SetCurrentTableId(tableId + 1);
    std::string name{this->FormatTableName(tableId)};
    //create file
    sharpen::FileChannelPtr channel{this->OpenChannel(name.data(),sharpen::FileAccessModel::All,sharpen::FileOpenModel::CreateNew)};
    sharpen::SstOption opt{this->comp_,this->filterBitsOfElement_,0,0};
    sharpen::SortedStringTable table{channel,opt};
    sharpen::SstBuildOption buildOpt{false,this->filterBitsOfElement_,this->blockSize_};
    table.Build(map.begin(),map.end(),buildOpt);
    try
    {
        this->AddToComponent(std::move(table),tableId,0);
    }
    catch(const std::exception&)
    {
        this->SetCurrentTableId(tableId);
        throw;
    }
    //this->SetPrevTableId(this->GetCurrentTableId());
    //cleanup tables
    this->imMems_.clear();
    std::size_t beginKey{this->GetPrevMemoryTableId()};
    std::size_t endKey{this->GetCurrentMemoryTableId()};
    while (beginKey != endKey)
    {
        name = this->FormatMemoryTableName(beginKey);
        try
        {
            sharpen::RemoveFile(name.c_str());
        }
        catch(const std::system_error &error)
        {
            if(sharpen::IsFatalError(error.code().value()))
            {
                std::terminate();
            }
            //ignore error
            static_cast<void>(error);
        }
        ++beginKey;
    }
    this->SetPrevMemoryTableId(beginKey);
}

void sharpen::LevelTable::Put(sharpen::ByteBuffer key,sharpen::ByteBuffer value)
{
    sharpen::WriteBatch batch;
    batch.Put(std::move(key),std::move(value));
    this->Action(std::move(batch));
}

void sharpen::LevelTable::Delete(sharpen::ByteBuffer key)
{
    sharpen::WriteBatch batch;
    batch.Delete(std::move(key));
    this->Action(std::move(batch));
}

void sharpen::LevelTable::Action(sharpen::WriteBatch batch)
{
    {
        this->levelLock_->LockRead();
        std::unique_lock<sharpen::AsyncReadWriteLock> lock{*this->levelLock_,std::adopt_lock};
        std::size_t increaseSize{0};
        for (auto begin = batch.Begin(),end = batch.End(); begin != end; ++begin)
        {
            increaseSize += begin->key_.GetSize();
            if(begin->type_ == sharpen::WriteBatch::ActionType::Put)
            {
                increaseSize += begin->value_.GetSize();
            }
        }
        this->mem_->Action(std::move(batch));
        this->usedMemory_->fetch_add(increaseSize,std::memory_order::memory_order_relaxed);
        if(this->usedMemory_->load() > this->maxSizeOfMem_)
        {
            this->levelLock_->UpgradeFromRead();
            if(!this->usedMemory_->load(std::memory_order::memory_order_relaxed))
            {
                return;
            }
            this->usedMemory_->store(0);
            std::unique_ptr<MemTable> imMem{std::move(this->mem_)};
            this->mem_ = this->MakeNewMemoryTable();
            this->imMems_.emplace_back(std::move(imMem));
            if(this->imMems_.size() == this->maxSizeOfImMems_)
            {
                this->DummpImmutableTables();
            }
        }
    }
}

sharpen::Optional<sharpen::ByteBuffer> sharpen::LevelTable::TryGet(const sharpen::ByteBuffer &key) const
{
    this->levelLock_->LockRead();
    std::unique_lock<sharpen::AsyncReadWriteLock> lock{*this->levelLock_,std::adopt_lock};
    //query memory table
    auto exist{this->mem_->Exist(key)};
    if(exist == sharpen::ExistStatus::Exist)
    {
        return this->mem_->Get(key);
    }
    else if(exist == sharpen::ExistStatus::Deleted)
    {
        return sharpen::EmptyOpt;
    }
    //query immutable tables
    for (auto begin = this->imMems_.rbegin(),end = this->imMems_.rend(); begin != end; ++begin)
    {
        exist = (*begin)->Exist(key);
        if (exist == sharpen::ExistStatus::Exist)
        {
            return (*begin)->Get(key);
        }
        else if(exist == sharpen::ExistStatus::Deleted)
        {
            return sharpen::EmptyOpt;
        }
    }
    //query conponents
    std::size_t maxLevel{this->GetMaxLevel()};
    for (std::size_t i = 0,count = maxLevel + 1; i != count; ++i)
    {
        const sharpen::LevelComponent *component{&this->GetComponent(i)};
        for (auto begin = component->ReverseBegin(),end = component->ReverseEnd(); begin != end; ++begin)
        {
            const sharpen::LevelView *view{&this->GetView(*begin)};
            sharpen::Optional<std::uint64_t> r{view->FindId(key)};
            if (r.Exist())
            {
                std::shared_ptr<sharpen::SortedStringTable> sst{this->LoadTableCache(r.Get())};
                sharpen::Optional<sharpen::ByteBuffer> val{sst->TryGet(key)};
                if(val.Exist())
                {
                    if(val.Get().Empty())
                    {
                        return sharpen::EmptyOpt;
                    }
                    return val;
                }
            }
        }
    }
    return sharpen::EmptyOpt;
}

sharpen::ExistStatus sharpen::LevelTable::Exist(const sharpen::ByteBuffer &key) const
{
    this->levelLock_->LockRead();
    std::unique_lock<sharpen::AsyncReadWriteLock> lock{*this->levelLock_,std::adopt_lock};
    //query memory table
    auto exist{this->mem_->Exist(key)};
    if(exist == sharpen::ExistStatus::Exist)
    {
        return sharpen::ExistStatus::Exist;
    }
    else if(exist == sharpen::ExistStatus::Deleted)
    {
        return sharpen::ExistStatus::Deleted;
    }
    //query immutable tables
    for (auto begin = this->imMems_.rbegin(),end = this->imMems_.rend(); begin != end; ++begin)
    {
        exist = (*begin)->Exist(key);
        if (exist == sharpen::ExistStatus::Exist)
        {
            return sharpen::ExistStatus::Exist;
        }
        else if(exist == sharpen::ExistStatus::Deleted)
        {
            return sharpen::ExistStatus::Deleted;
        }
    }
    //query conponents
    std::size_t maxLevel{this->GetMaxLevel()};
    for (std::size_t i = 0,count = maxLevel + 1; i != count; ++i)
    {
        const sharpen::LevelComponent *component{&this->GetComponent(i)};
        for (auto begin = component->ReverseBegin(),end = component->ReverseEnd(); begin != end; ++begin)
        {
            const sharpen::LevelView *view{&this->GetView(*begin)};
            auto r{view->FindId(key)};
            if (r.Exist())
            {
                auto sst{this->LoadTableCache(r.Get())};
                auto val{sst->TryGet(key)};
                if(val.Exist())
                {
                    if(val.Get().Empty())
                    {
                        return sharpen::ExistStatus::Deleted;
                    }
                    return sharpen::ExistStatus::Exist;
                }
            }
        }
    }
    return sharpen::ExistStatus::NotExist;
}

std::uint64_t sharpen::LevelTable::GetTableSize() const
{
    this->levelLock_->LockRead();
    std::unique_lock<sharpen::AsyncReadWriteLock> lock{*this->levelLock_,std::adopt_lock};
    //memory table
    std::uint64_t size{this->usedMemory_->load()};
    //immutable tables
    for (auto begin = this->imMems_.rbegin(),end = this->imMems_.rend(); begin != end; ++begin)
    {
        for (auto kb = (*begin)->Begin(),ke = (*begin)->End(); kb != ke; ++kb)
        {
            size += kb->first.GetSize();
            if(!kb->second.IsDeleted())
            {
                size += kb->second.Value().GetSize();
            }   
        }
    }
    //components
    std::size_t maxLevel{this->GetMaxLevel()};
    for (std::size_t i = 0,count = maxLevel + 1; i != count; ++i)
    {
        const sharpen::LevelComponent *component{&this->GetComponent(i)};
        for (auto begin = component->ReverseBegin(),end = component->ReverseEnd(); begin != end; ++begin)
        {
            const sharpen::LevelView *view{&this->GetView(*begin)};
            for (auto vb = view->Begin(),ve = view->End(); vb != ve; ++vb)
            {
                auto tptr{this->LoadTableFromCache(vb->GetId())};
                if(tptr)
                {
                    size += tptr->GetTableSize();
                    continue;
                }
                std::string name{this->FormatTableName(vb->GetId())};
                if(sharpen::ExistFile(name.data()))
                {
                    sharpen::FileChannelPtr channel{this->OpenChannel(name.data(),sharpen::FileAccessModel::Read,sharpen::FileOpenModel::Open)};
                    size += channel->GetFileSize();
                }   
            }
        }
    }
    return size;
}

void sharpen::LevelTable::Destory()
{
    this->mem_.reset();
    this->imMems_.clear();
    this->tableCaches_.Release();
    for (std::size_t i = this->GetPrevMemoryTableId(),end = this->GetCurrentMemoryTableId() + 1; i != end; ++i)
    {
        std::string name{this->FormatMemoryTableName(i)};
        try
        {
            sharpen::RemoveFile(name.c_str());
        }
        catch(const std::system_error &error)
        {
            if(sharpen::IsFatalError(error.code().value()))
            {
                std::terminate();
            }
            //ignore error
            static_cast<void>(error);
        }
    }
    for (std::size_t i = 0,end = this->GetCurrentTableId() + 1; i != end; ++i)
    {
        std::string name{this->FormatTableName(i)};
        if(sharpen::ExistFile(name.c_str()))
        {
            try
            {
                sharpen::RemoveFile(name.c_str());
            }
            catch(const std::system_error &error)
            {
                if(sharpen::IsFatalError(error.code().value()))
                {
                    std::terminate();
                }
                //ignore error
                static_cast<void>(error);
            }
        }
    }
    this->manifest_.release();
    std::string name{this->FormatManifestName()};
    try
    {
        sharpen::RemoveFile(name.c_str());
    }
    catch(const std::system_error &error)
    {
        if(sharpen::IsFatalError(error.code().value()))
        {
            std::terminate();
        }
        //ignore error
        static_cast<void>(error);
    }
}