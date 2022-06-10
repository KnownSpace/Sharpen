#pragma once
#ifndef _SHARPEN_LEVELTABLE_HPP
#define _SHARPEN_LEVELTABLE_HPP

#include <set>

#include "LevelTableOption.hpp"
#include "LevelTableScanner.hpp"

namespace sharpen
{
    class LevelTable:public sharpen::Noncopyable
    {
    public:
        using MemTable = sharpen::MemoryTable<sharpen::BinaryLogger,sharpen::MemoryTableComparator>;
    private:
        using Self = sharpen::LevelTable;
        using ComponentMap = std::map<std::uint64_t,sharpen::LevelComponent>;
        using ViewMap = std::map<std::uint64_t,sharpen::LevelView>;
        using Comparator = std::int32_t(*)(const sharpen::ByteBuffer&,const sharpen::ByteBuffer&);
        using FileGenerator = sharpen::FileChannelPtr(*)(const char*,sharpen::FileAccessModel,sharpen::FileOpenModel);    

        //manifest
        //component: l{binaryLevel}
        //view: v{binaryId}
        //max level: ml
        //current table id: ctid
        //current view id: cvid
        //current memory table id: cmid
        //prev table id: ptid
        //prev view id:pvid
        //prev memory table id:pmid
        static sharpen::ByteBuffer currentTableIdKey_;
        static sharpen::ByteBuffer currentViewIdKey_;
        static sharpen::ByteBuffer currentMemTableIdKey_;
        static sharpen::ByteBuffer maxLevelKey_;
        static sharpen::ByteBuffer prevTableIdKey_;
        static sharpen::ByteBuffer prevViewIdKey_;
        static sharpen::ByteBuffer prevMemTableIdKey_;
        static std::once_flag keyFlag_;

        //format
        //table: {tableName}_{id}.{tableExtName}
        //wal: {tableName}_{id}.{walExtName}
        //manifest: {tableName}_manifest.{walExtName}
        std::string tableName_;
        std::string tableExtName_;
        std::string walExtName_;
        //maps
        mutable ComponentMap componentMap_;
        mutable ViewMap viewMap_;
        //table cache
        mutable sharpen::SegmentedCircleCache<sharpen::SortedStringTable> tableCaches_;
        //file generator
        FileGenerator fileGenerator_;
        //memory table
        std::unique_ptr<MemTable> mem_;
        //manifest
        std::unique_ptr<MemTable> manifest_;
        //immutable memory tables
        std::vector<std::unique_ptr<MemTable>> imMems_;
        //locks
        mutable std::unique_ptr<sharpen::AsyncReadWriteLock> levelLock_;
        mutable std::unique_ptr<sharpen::AsyncReadWriteLock> viewLock_;
        mutable std::unique_ptr<sharpen::AsyncReadWriteLock> componentLock_;
        //comparator
        Comparator comp_;
        //config
        std::size_t maxViewOfComponent_;
        std::size_t maxTableOfComponent_;
        std::size_t blockCacheSize_;
        std::size_t filterBitsOfElement_;
        std::size_t maxSizeOfMem_;
        std::size_t maxSizeOfImMems_;
        std::size_t blockSize_;
        std::unique_ptr<std::atomic_size_t> usedMemory_;
        //engine
        sharpen::EventEngine *engine_;

        static sharpen::ByteBuffer GetViewKey(std::uint64_t id);

        static sharpen::ByteBuffer GetComponetKey(std::uint64_t level);

        static void InitManifestKeys();

        std::uint64_t GetCurrentTableId() const noexcept;

        std::uint64_t GetCurrentViewId() const noexcept;

        std::uint64_t GetCurrentMemoryTableId() const noexcept;

        std::uint64_t GetPrevTableId() const noexcept;

        std::uint64_t GetPrevViewId() const noexcept;

        std::uint64_t GetPrevMemoryTableId() const noexcept;

        void SetCurrentTableId(std::uint64_t id);

        void SetCurrentViewId(std::uint64_t id);

        void SetMaxLevel(std::uint64_t level);

        void SetCurrentMemoryTableId(std::uint64_t id);

        void SetPrevTableId(std::uint64_t id);

        void SetPrevViewId(std::uint64_t id);

        void SetPrevMemoryTableId(std::uint64_t id);

        sharpen::FileChannelPtr OpenChannel(const char *name,sharpen::FileAccessModel accessModel,sharpen::FileOpenModel openModel) const;

        sharpen::LevelView &GetView(std::uint64_t id);

        const sharpen::LevelView &GetView(std::uint64_t id) const;

        void SaveView(std::uint64_t id,const sharpen::LevelView &view);

        sharpen::LevelComponent &GetComponent(std::uint64_t id);

        const sharpen::LevelComponent &GetComponent(std::uint64_t id) const;

        void SaveComponent(std::uint64_t id,const sharpen::LevelComponent &component);

        std::size_t GetTableCount(const sharpen::LevelComponent &component) const;

        std::string FormatTableName(std::uint64_t tableId) const;

        std::string FormatMemoryTableName(std::uint64_t tableId) const;

        std::string FormatManifestName() const;

        sharpen::SortedStringTable MergeTables(const sharpen::LevelComponent &component,std::uint64_t newTableId,bool eraseDeleted,sharpen::Optional<sharpen::SortedStringTable> appendTable);

        void AddToComponent(sharpen::SortedStringTable table,std::uint64_t tableId,std::uint64_t componentId);

        sharpen::SortedStringTable LoadTable(std::uint64_t id) const;

        void DeleteTableFromCache(std::uint64_t id);

        void DeleteTable(std::uint64_t id);

        std::shared_ptr<sharpen::SortedStringTable> LoadTableFromCache(std::uint64_t id) const;

        std::shared_ptr<sharpen::SortedStringTable> LoadTableCache(std::uint64_t id) const;

        std::unique_ptr<MemTable> MakeNewMemoryTable();

        void DummpImmutableTables();

        void InitImmutableTables();

        void InitMemoryTable();

        void InitManifest();

        void GcTables();

        void GcViews();
    public:

        LevelTable(sharpen::EventEngine &engine,const std::string &tableName,const std::string &tableExtName)
            :LevelTable(engine,tableName,tableExtName,sharpen::LevelTableOption{})
        {}

        LevelTable(sharpen::EventEngine &engine,const std::string &tableName,const std::string &tableExtName,const sharpen::LevelTableOption &opt)
            :LevelTable(engine,tableName,tableExtName,"wal",opt)
        {}

        LevelTable(sharpen::EventEngine &engine,const std::string &tableName,const std::string &tableExtName,const std::string &walExtName,const sharpen::LevelTableOption &opt);
    
        LevelTable(Self &&other) noexcept;
    
        Self &operator=(Self &&other) noexcept;
    
        ~LevelTable() noexcept = default;

        std::int32_t CompareKeys(const sharpen::ByteBuffer &left,const sharpen::ByteBuffer &right) const noexcept;

        void Put(sharpen::ByteBuffer key,sharpen::ByteBuffer value);

        void Delete(sharpen::ByteBuffer key);

        void Action(sharpen::WriteBatch batch);

        sharpen::ExistStatus Exist(const sharpen::ByteBuffer &key) const;

        sharpen::Optional<sharpen::ByteBuffer> TryGet(const sharpen::ByteBuffer &key) const;

        inline sharpen::ByteBuffer Get(const sharpen::ByteBuffer &key) const
        {
            auto r{this->TryGet(key)};
            if(!r.Exist())
            {
                throw std::out_of_range("key doesn't exist");
            }
            return r.Get();
        }

        //for range query
        //you should lock level lock(S) first
        inline sharpen::AsyncReadWriteLock &GetLevelLock() const noexcept
        {
            return *this->levelLock_;
        }

        //for range query
        //you should lock level lock(S) first
        std::uint64_t GetMaxLevel() const noexcept;

        //for range query
        //you should lock level lock(S) first
        inline std::shared_ptr<const sharpen::SortedStringTable> GetTable(std::uint64_t id) const
        {
            return this->LoadTableCache(id);
        }

        inline sharpen::SortedStringTable GetTableCopy(std::uint64_t id) const
        {
            return this->LoadTable(id);
        }

        //for range query
        //you should lock level lock(S) first
        template<typename _InsertIterator,typename _Check = decltype(*std::declval<_InsertIterator&>()++ = static_cast<const sharpen::LevelView*>(nullptr))>
        void GetAllViewOfComponent(std::uint64_t level,_InsertIterator inserter) const
        {
            const sharpen::LevelComponent *component{&this->GetComponent(level)};
            for (auto begin = component->Begin(),end = component->End(); begin != end; ++begin)
            {
                *inserter++ = &this->GetView(*begin);   
            }
        }

        //for range query
        //you should lock level lock(S) first
        inline const MemTable &GetMemoryTable() const noexcept
        {
            return *this->mem_;
        }

        //for range query
        //you should lock level lock(S) first
        template<typename _InsertIterator,typename _Check = decltype(*std::declval<_InsertIterator&>()++ = static_cast<const MemTable*>(nullptr))>
        inline void GetAllImmutableTables(_InsertIterator inserter)
        {
            for (auto begin = this->imMems_.begin(),end = this->imMems_.end(); begin != end; ++begin)
            {
                *inserter++ = begin->get();
            }
        }

        std::uint64_t GetTableSize() const;

        //for range query
        //you should lock level lock(S) first
        template<typename _InsertIterator,typename _Checker = decltype(*std::declval<_InsertIterator&>()++ = std::declval<sharpen::LevelViewItem&>())>
        inline void TableScan(_InsertIterator inserter) const
        {
            std::map<std::uint64_t,std::size_t> viewStatus;
            sharpen::Optional<sharpen::LevelViewItem> selectedItem;
            std::uint64_t selectedView{0};
            std::uint64_t maxLevel{this->GetMaxLevel()};
            std::set<std::uint64_t> emptyComponents;
            std::size_t levelBoundary{maxLevel + 1};
            while (emptyComponents.size() != levelBoundary)
            {
                for (std::size_t i = maxLevel;; --i)
                {
                    const sharpen::LevelComponent *component{&this->GetComponent(i)};
                    std::size_t emptyViews{0};
                    if(!component->Empty())
                    {
                        for (auto begin = component->Begin(),end = component->End(); begin != end; ++begin)
                        {
                            const sharpen::LevelView *view{&this->GetView(*begin)};
                            if(!view->Empty())
                            {
                                std::size_t skip{0};
                                auto ite = viewStatus.find(*begin);
                                if(ite != viewStatus.end())
                                {
                                    skip = ite->second;
                                }
                                else
                                {
                                    viewStatus.emplace(*begin,skip);
                                }
                                if(skip != view->GetSize())
                                {
                                    auto tableIte = sharpen::IteratorForward(view->Begin(),skip);
                                    if(!selectedItem.Exist() || this->CompareKeys(tableIte->BeginKey(),selectedItem.Get().BeginKey()) != -1)
                                    {
                                        selectedItem.Construct(*tableIte);
                                        selectedView = *begin;
                                    }
                                    continue;
                                }
                            }
                            emptyViews += 1;
                        }
                    }
                    if(emptyViews == component->GetSize())
                    {
                        emptyComponents.emplace(i);
                    }
                    if(!i)
                    {
                        break;
                    }
                }
                if(selectedItem.Exist())
                {
                    *inserter++ = std::move(selectedItem.Get());
                    selectedItem.Reset();
                    viewStatus[selectedView] += 1;
                    selectedView = 0;
                }
            }
        }

        //for range query
        //you should lock level lock(S) first
        template<typename _InsertIterator,typename _Checker = decltype(*std::declval<_InsertIterator&>()++ = std::declval<sharpen::LevelViewItem&>())>
        inline void TableScan(_InsertIterator inserter,const sharpen::ByteBuffer &beginKey,const sharpen::ByteBuffer &endKey) const
        {
            std::map<std::uint64_t,std::pair<std::size_t,std::size_t>> viewStatus;
            sharpen::Optional<sharpen::LevelViewItem> selectedItem;
            std::uint64_t selectedView{0};
            std::uint64_t maxLevel{this->GetMaxLevel()};
            std::set<std::uint64_t> emptyComponents;
            //set query range
            for (std::size_t i = 0,count = maxLevel + 1; i != count; ++i)
            {
                const sharpen::LevelComponent *component{&this->GetComponent(i)};
                if(!component->Empty())
                { 
                    for (auto begin = component->Begin(),end = component->End(); begin != end; ++begin)
                    {
                        const sharpen::LevelView *view{&this->GetView(*begin)};
                        if(!view->Empty())
                        {
                            auto tableBegin = view->Begin(),tableEnd = view->End();
                            std::size_t beginRange{0};
                            std::size_t endRange{0};
                            for (;tableBegin != tableEnd; ++tableBegin,++beginRange)
                            {
                                std::int32_t r{this->CompareKeys(beginKey,tableBegin->EndKey())};
                                if(r != -1)
                                {
                                    break;
                                }
                            }
                            endRange = beginRange + 1;
                            for (;tableBegin != tableEnd; ++tableBegin,++endRange)
                            {
                                std::int32_t r{this->CompareKeys(endKey,tableBegin->EndKey())};
                                if(r != -1)
                                {
                                    break;
                                }
                            }
                            viewStatus.emplace(*begin,std::pair<std::size_t,std::size_t>{beginRange,endRange});
                        }
                    }   
                }
            }
            std::size_t levelBoundary{maxLevel+1};
            while (emptyComponents.size() != levelBoundary)
            {
                for (std::size_t i = maxLevel;; --i)
                {
                    const sharpen::LevelComponent *component{&this->GetComponent(i)};
                    std::size_t emptyViews{0};
                    if(!component->Empty())
                    {
                        for (auto begin = component->Begin(),end = component->End(); begin != end; ++begin)
                        {
                            const sharpen::LevelView *view{&this->GetView(*begin)};
                            if(!view->Empty())
                            {
                                std::size_t skip{0};
                                auto ite = viewStatus.find(*begin);
                                assert(ite != viewStatus.end());
                                skip = ite->second.first;
                                if(skip != ite->second.second)
                                {
                                    auto tableIte = sharpen::IteratorForward(view->Begin(),skip);
                                    if(!selectedItem.Exist() || this->CompareKeys(tableIte->BeginKey(),selectedItem.Get().BeginKey()) != -1)
                                    {
                                        selectedItem.Construct(*tableIte);
                                        selectedView = *begin;
                                    }
                                    continue;
                                }
                            }
                            emptyViews += 1;
                        }
                    }
                    if(emptyViews == component->GetSize())
                    {
                        emptyComponents.emplace(i);
                    }
                    if(!i)
                    {
                        break;
                    }
                }
                if(selectedItem.Exist())
                {
                    *inserter++ = std::move(selectedItem.Get());
                    selectedItem.Reset();
                    viewStatus[selectedView].first += 1;
                    selectedView = 0;
                }
            }
        }

        void Destory();

        inline sharpen::LevelTableScanner Scan(bool useCache) const
        {
            sharpen::LevelTableScanner scanner{*this};
            if(!useCache)
            {
                scanner.DisableCache();
            }
            return scanner;
        }

        inline sharpen::LevelTableScanner Scan(const sharpen::ByteBuffer &beginKey,const sharpen::ByteBuffer &endKey,bool useCache) const
        {
            sharpen::LevelTableScanner scanner{*this,beginKey,endKey};
            if(!useCache)
            {
                scanner.DisableCache();
            }
            return scanner;
        }
    };
}

#endif