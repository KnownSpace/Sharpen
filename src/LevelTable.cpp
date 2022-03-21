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

sharpen::ByteBuffer sharpen::LevelTable::GetViewKey(sharpen::Uint64 id)
{
    sharpen::ByteBuffer buf{1 + sizeof(id)};
    buf[0] = 'v';
    std::memcpy(buf.Data() + 1,&id,sizeof(id));
    return buf;
}

sharpen::ByteBuffer sharpen::LevelTable::GetComponetKey(sharpen::Uint64 level)
{
    sharpen::ByteBuffer buf{1 + sizeof(level)};
    buf[0] = 'l';
    std::memcpy(buf.Data() + 1,&level,sizeof(level));
    return buf;
}

sharpen::Uint64 sharpen::LevelTable::GetCurrentTableId() const noexcept
{
    return this->manifest_->Get(this->currentTableIdKey_).As<sharpen::Uint64>();
}

sharpen::Uint64 sharpen::LevelTable::GetCurrentViewId() const noexcept
{
    return this->manifest_->Get(this->currentViewIdKey_).As<sharpen::Uint64>();
}

sharpen::Uint64 sharpen::LevelTable::GetCurrentMemoryTableId() const noexcept
{
    return this->manifest_->Get(this->currentMemTableIdKey_).As<sharpen::Uint64>();
}

sharpen::Uint64 sharpen::LevelTable::GetMaxLevel() const noexcept
{
    return this->manifest_->Get(this->maxLevelKey_).As<sharpen::Uint64>();
}

sharpen::Uint64 sharpen::LevelTable::GetPrevTableId() const noexcept
{
    return this->manifest_->Get(this->prevTableIdKey_).As<sharpen::Uint64>();
}

sharpen::Uint64 sharpen::LevelTable::GetPrevViewId() const noexcept
{
    return this->manifest_->Get(this->prevViewIdKey_).As<sharpen::Uint64>();
}

sharpen::Uint64 sharpen::LevelTable::GetPrevMemoryTableId() const noexcept
{
    return this->manifest_->Get(this->prevMemTableIdKey_).As<sharpen::Uint64>();
}

void sharpen::LevelTable::SetCurrentTableId(sharpen::Uint64 id)
{
    sharpen::ByteBuffer buf{sizeof(id)};
    buf.As<sharpen::Uint64>() = id;
    this->manifest_->Put(this->currentTableIdKey_,std::move(buf));
}

void sharpen::LevelTable::SetCurrentViewId(sharpen::Uint64 id)
{
    sharpen::ByteBuffer buf{sizeof(id)};
    buf.As<sharpen::Uint64>() = id;
    this->manifest_->Put(this->currentViewIdKey_,std::move(buf));
}

void sharpen::LevelTable::SetCurrentMemoryTableId(sharpen::Uint64 id)
{
    sharpen::ByteBuffer buf{sizeof(id)};
    buf.As<sharpen::Uint64>() = id;
    this->manifest_->Put(this->currentMemTableIdKey_,std::move(buf));
}

void sharpen::LevelTable::SetMaxLevel(sharpen::Uint64 level)
{
    sharpen::ByteBuffer buf{sizeof(level)};
    buf.As<sharpen::Uint64>() = level;
    this->manifest_->Put(this->maxLevelKey_,std::move(buf));
}

void sharpen::LevelTable::SetPrevTableId(sharpen::Uint64 id)
{
    sharpen::ByteBuffer buf{sizeof(id)};
    buf.As<sharpen::Uint64>() = id;
    this->manifest_->Put(this->prevTableIdKey_,std::move(buf));
}

void sharpen::LevelTable::SetPrevViewId(sharpen::Uint64 id)
{
    sharpen::ByteBuffer buf{sizeof(id)};
    buf.As<sharpen::Uint64>() = id;
    this->manifest_->Put(this->prevViewIdKey_,std::move(buf));
}

void sharpen::LevelTable::SetPrevMemoryTableId(sharpen::Uint64 id)
{
    sharpen::ByteBuffer buf{sizeof(id)};
    buf.As<sharpen::Uint64>() = id;
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
    sharpen::ByteBuffer buf{sizeof(sharpen::Uint64)};
    buf.As<sharpen::Uint64>() = 0;
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

sharpen::LevelView &sharpen::LevelTable::GetView(sharpen::Uint64 id)
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

const sharpen::LevelView &sharpen::LevelTable::GetView(sharpen::Uint64 id) const
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

void sharpen::LevelTable::SaveView(sharpen::Uint64 id,const sharpen::LevelView &view)
{
    sharpen::ByteBuffer buf;
    view.StoreTo(buf);
    sharpen::ByteBuffer key{this->GetViewKey(id)};
    this->manifest_->Put(std::move(key),std::move(buf));
}

sharpen::LevelComponent &sharpen::LevelTable::GetComponent(sharpen::Uint64 id)
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

const sharpen::LevelComponent &sharpen::LevelTable::GetComponent(sharpen::Uint64 id) const
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

void sharpen::LevelTable::SaveComponent(sharpen::Uint64 id,const sharpen::LevelComponent &component)
{
    sharpen::ByteBuffer buf;
    component.StoreTo(buf);
    sharpen::ByteBuffer key{this->GetComponetKey(id)};
    this->manifest_->Put(std::move(key),std::move(buf));
}

sharpen::Size sharpen::LevelTable::GetTableCount(const sharpen::LevelComponent &component) const
{
    sharpen::Size count{0};
    for (auto begin = component.Begin(),end = component.End(); begin != end; ++begin)
    {
        const sharpen::LevelView *view{&this->GetView(*begin)};
        count += view->GetSize();
    }
    return count;
}

std::string sharpen::LevelTable::FormatTableName(sharpen::Uint64 id) const
{
    //{tableName}_{id}.{tableExtName}
    std::string name;
    name.resize(this->tableName_.size() + 1 + 44 + 1 + this->tableExtName_.size() + 1,0);
    //copy table name
    std::memcpy(const_cast<char*>(name.data()),this->tableName_.data(),this->tableName_.size());
    //copy '_'
    name[this->tableName_.size()] = '_';
    //copy id
    sharpen::Size size{sharpen::Itoa(id,10,const_cast<char*>(name.data()) + this->tableName_.size() + 1)};
    //copy .
    name[this->tableName_.size() + 1 + size] = '.';
    //copy tableExtName
    std::memcpy(const_cast<char*>(name.data())  + this->tableName_.size() + 1 + size + 1,this->tableExtName_.data(),this->tableExtName_.size());
    return name;
}

std::string sharpen::LevelTable::FormatMemoryTableName(sharpen::Uint64 id) const
{
    //{tableName}_{id}.{walExtName}
    std::string name;
    name.resize(this->tableName_.size() + 1 + 44 + 1 + this->tableExtName_.size() + 1,0);
    //copy table name
    std::memcpy(const_cast<char*>(name.data()),this->tableName_.data(),this->tableName_.size());
    //copy '_'
    name[this->tableName_.size()] = '_';
    //copy id
    sharpen::Size size{sharpen::Itoa(id,10,const_cast<char*>(name.data()) + this->tableName_.size() + 1)};
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

sharpen::SortedStringTable sharpen::LevelTable::MergeTables(const sharpen::LevelComponent &component,sharpen::Uint64 newTableId,bool eraseDeleted,sharpen::Optional<sharpen::SortedStringTable> appendTable)
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
            sharpen::Uint64 viewId{*component.ReverseBegin()};
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

void sharpen::LevelTable::AddToComponent(sharpen::SortedStringTable table,sharpen::Uint64 tableId,sharpen::Uint64 componentId)
{
    //load component
    sharpen::LevelComponent *component{&this->GetComponent(componentId)};
    //load level
    sharpen::Uint64 level{this->GetMaxLevel()};
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
            sharpen::Uint64 viewId{this->GetCurrentViewId()};
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
        sharpen::Size tableNum{this->GetTableCount(*component)};
        //if table number > max
        if (this->maxTableOfComponent_ != 0 && tableNum == this->maxTableOfComponent_)
        {
            //copy old views
            std::vector<sharpen::Uint64> oldViews{component->Begin(),component->End()};
            //merge tables
            sharpen::Uint64 newTableId{this->GetCurrentTableId()};
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
        sharpen::Uint64 viewId{*component->ReverseBegin()};
        view = &this->GetView(viewId);
        bool r{view->TryPut(firstKey,lastKey,tableId)};
        if(!r)
        {
            //if view number > max 
            if(this->maxViewOfComponent_ != 0 && component->GetSize() == this->maxViewOfComponent_)
            {
                //copy old views
                std::vector<sharpen::Uint64> oldViews{component->Begin(),component->End()};
                //merge tables
                sharpen::Uint64 newTableId{this->GetCurrentTableId()};
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

sharpen::SortedStringTable sharpen::LevelTable::LoadTable(sharpen::Uint64 id) const
{
    std::string name{this->FormatTableName(id)};
    sharpen::FileChannelPtr channel{this->OpenChannel(name.data(),sharpen::FileAccessModel::Read,sharpen::FileOpenModel::CreateOrOpen)};
    sharpen::SstOption opt{this->comp_,this->filterBitsOfElement_,this->blockCacheSize_,this->blockCacheSize_};
    return {channel,opt};
}

void sharpen::LevelTable::DeleteTableFromCache(sharpen::Uint64 id)
{
    this->tableCaches_.Delete(reinterpret_cast<char*>(&id),reinterpret_cast<char*>(&id) + sizeof(id));
}

void sharpen::LevelTable::DeleteTable(sharpen::Uint64 id)
{
    this->DeleteTableFromCache(id);
    std::string name{this->FormatTableName(id)};
    if(sharpen::ExistFile(name.data()))
    {
        sharpen::RemoveFile(name.data());
    }
}

std::shared_ptr<sharpen::SortedStringTable> sharpen::LevelTable::LoadTableFromCache(sharpen::Uint64 id) const
{
    return this->tableCaches_.Get(reinterpret_cast<char*>(&id),reinterpret_cast<char*>(&id) + sizeof(id));
}

std::shared_ptr<sharpen::SortedStringTable> sharpen::LevelTable::LoadTableCache(sharpen::Uint64 id) const
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
    sharpen::Uint64 memId{this->GetCurrentMemoryTableId()};
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
    sharpen::Uint64 beginKey{this->GetPrevMemoryTableId()};
    sharpen::Uint64 endKey{this->GetCurrentMemoryTableId()};
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
    sharpen::Uint64 memId{this->GetCurrentMemoryTableId()};
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
    sharpen::Uint64 begin{this->GetPrevTableId()};
    sharpen::Uint64 end{this->GetCurrentTableId()};
    if (begin != end)
    {
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
    sharpen::Uint64 begin{this->GetPrevViewId()};
    sharpen::Uint64 end{this->GetCurrentViewId()};
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
    sharpen::Uint64 tableId{this->GetCurrentTableId()};
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
    sharpen::Size beginKey{this->GetPrevMemoryTableId()};
    sharpen::Size endKey{this->GetCurrentMemoryTableId()};
    while (beginKey != endKey)
    {
        name = this->FormatMemoryTableName(beginKey);
        try
        {
            sharpen::RemoveFile(name.c_str());
        }
        catch(const std::exception &ignore)
        {
            //ignore error
            static_cast<void>(ignore);
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
        sharpen::Size increaseSize{0};
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
    sharpen::Size maxLevel{this->GetMaxLevel()};
    for (sharpen::Size i = 0,count = maxLevel + 1; i != count; ++i)
    {
        const sharpen::LevelComponent *component{&this->GetComponent(i)};
        for (auto begin = component->ReverseBegin(),end = component->ReverseEnd(); begin != end; ++begin)
        {
            const sharpen::LevelView *view{&this->GetView(*begin)};
            sharpen::Optional<sharpen::Uint64> r{view->FindId(key)};
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
    sharpen::Size maxLevel{this->GetMaxLevel()};
    for (sharpen::Size i = 0,count = maxLevel + 1; i != count; ++i)
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

sharpen::Uint64 sharpen::LevelTable::GetTableSize() const
{
    this->levelLock_->LockRead();
    std::unique_lock<sharpen::AsyncReadWriteLock> lock{*this->levelLock_,std::adopt_lock};
    //memory table
    sharpen::Uint64 size{this->usedMemory_->load()};
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
    sharpen::Size maxLevel{this->GetMaxLevel()};
    for (sharpen::Size i = 0,count = maxLevel + 1; i != count; ++i)
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