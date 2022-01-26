#pragma once
#ifndef _SHARPEN_MEMORYTABLE_HPP
#define _SHARPEN_MEMORYTABLE_HPP

/*
+-------
|
+-----
*/

#include <stdexcept>
#include <map>
#include <cassert>

#include "ByteBuffer.hpp"
#include "CompressedPair.hpp"
#include "MemoryTableConcepts.hpp"
#include "Noncopyable.hpp"
#include "Optional.hpp"

namespace sharpen
{   
    class MemoryTableItem
    {
    private:

        using Self = sharpen::MemoryTableItem;

        sharpen::Optional<sharpen::ByteBuffer> value_;
    public:
        MemoryTableItem() = default;

        explicit MemoryTableItem(sharpen::ByteBuffer value)
            :value_(std::move(value))
        {}
    
        MemoryTableItem(const Self &other) = default;
    
        MemoryTableItem(Self &&other) noexcept = default;
    
        inline Self &operator=(const Self &other)
        {
            Self tmp{other};
            std::swap(tmp,*this);
            return *this;
        }
    
        Self &operator=(Self &&other) noexcept = default;
    
        ~MemoryTableItem() noexcept = default;

        inline sharpen::ByteBuffer &Value() noexcept
        {
            assert(this->value_.Exist());
            return this->value_.Get();
        }

        inline const sharpen::ByteBuffer &Value() const noexcept
        {
            assert(this->value_.Exist());
            return this->value_.Get();
        }

        inline bool IsDeleted() const noexcept
        {
            return !this->value_.Exist();
        }

        void Delete() noexcept
        {
            this->value_.Reset();
        }
    };

    template<typename _Logger,typename _Check = void>
    class InternalMemoryTable;

    template<typename _Logger>
    class InternalMemoryTable<_Logger,sharpen::EnableIf<sharpen::IsMemoryTableLogger<_Logger>::Value>>:public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::InternalMemoryTable<_Logger,sharpen::EnableIf<sharpen::IsMemoryTableLogger<_Logger>::Value>>;
        using StoreItem = sharpen::MemoryTableItem;
        using MapType = std::map<sharpen::ByteBuffer,StoreItem>;
        using ConstIterator = typename MapType::const_iterator;
    
        sharpen::CompressedPair<_Logger,MapType> pair_;

        _Logger &Logger() noexcept
        {
            return this->pair_.First();
        }

        const _Logger &Logger() const noexcept
        {
            return this->pair_.First();
        }

        MapType &Map() noexcept
        {
            return this->pair_.Second();
        }
        
        const MapType &Map() const noexcept
        {
            return this->pair_.Second();
        }

        void InternalPut(sharpen::ByteBuffer key,sharpen::ByteBuffer value)
        {
            this->Map()[std::move(key)] = StoreItem{std::move(value)};
        }

        void InternalDelete(sharpen::ByteBuffer key)
        {
            auto ite = this->Map().find(key);
            if(ite != this->Map().end())
            {
                ite->second.Delete();
            }
            else
            {
                this->Map().emplace(std::move(key),StoreItem{});
            }
        }

        void InternalAction(sharpen::WriteBatch &batch)
        {
            for (auto begin = batch.Begin(),end = batch.End(); begin != end; ++begin)
            {
                if (begin->type_ == sharpen::WriteBatch::ActionType::Put)
                {
                    this->InternalPut(std::move(begin->key_),std::move(begin->value_));
                }
                else
                {
                    this->InternalDelete(std::move(begin->key_));
                }
            }
            batch.Clear();
        }

        void InternalAction(const sharpen::WriteBatch &batch)
        {
            for (auto begin = batch.Begin(),end = batch.End(); begin != end; ++begin)
            {
                if (begin->type_ == sharpen::WriteBatch::ActionType::Put)
                {
                    this->InternalPut(begin->key_,begin->value_);
                }
                else
                {
                    this->InternalDelete(begin->key_);
                }
            }
        }
    public:

        enum class ExistStatus
        {
            Exist,
            Deleted,
            NotFound
        };
    
        template<typename ..._Args,typename _Check = decltype(_Logger{std::declval<_Args>()...})>
        explicit InternalMemoryTable(_Args &&...args)
            :pair_(_Logger{std::forward<_Args>(args)...},MapType{})
        {}
    
        InternalMemoryTable(Self &&other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->pair_ = std::move(other.pair_);
            }
            return *this;
        }
    
        ~InternalMemoryTable() noexcept = default;

        inline void Restore()
        {
            auto batchs = this->Logger().GetWriteBatchs();
            for (auto begin = batchs.begin(),end = batchs.end(); begin != end; ++begin)
            {
                sharpen::WriteBatch &batch = *begin;
                this->InternalAction(batch);
            }
        }

        inline void Action(sharpen::WriteBatch &batch)
        {
            this->Logger().Log(batch);
            this->InternalAction(batch);
        }

        inline void Action(sharpen::WriteBatch &&batch)
        {
            this->Action(batch);
        }

        inline void Action(const sharpen::WriteBatch &batch)
        {
            this->Logger().Log(batch);
            this->InternalAction(batch);
        }

        inline void Put(sharpen::ByteBuffer key,sharpen::ByteBuffer value)
        {
            sharpen::WriteBatch batch;
            batch.Put(std::move(key),std::move(value));
            this->Action(std::move(batch));
        }

        inline void Delete(sharpen::ByteBuffer key)
        {
            sharpen::WriteBatch batch;
            batch.Delete(std::move(key));
            this->Action(std::move(batch));
        }

        void ClearLogs()
        {
            this->Logger().Clear();
        }

        void RemoveLogs()
        {
            this->Logger().Remove();
        }

        Self::ExistStatus Exist(const sharpen::ByteBuffer &key) const noexcept
        {
            auto ite = this->Map().find(key);
            if(ite == this->Map().end())
            {
                return Self::ExistStatus::NotFound;
            }
            else if(ite->second.IsDeleted())
            {
                return Self::ExistStatus::Deleted;
            }
            return Self::ExistStatus::Exist;
        }

        sharpen::ByteBuffer &Get(const sharpen::ByteBuffer &key)
        {
            auto &item = this->Map().at(key);
            if(item.IsDeleted())
            {
                throw std::out_of_range("key doesn't exists");
            }
            return item.Value();
        }

        const sharpen::ByteBuffer &Get(const sharpen::ByteBuffer &key) const
        {
            auto &item = this->Map().at(key);
            if(item.deleteTag_)
            {
                throw std::out_of_range("key doesn't exists");
            }
            return item.value_;
        }

        sharpen::ByteBuffer &operator[](const sharpen::ByteBuffer &key)
        {
            return this->Get(key);
        }

        const sharpen::ByteBuffer &operator[](const sharpen::ByteBuffer &key) const
        {
            return this->Get(key);
        }

        inline ConstIterator Begin() const noexcept
        {
            return this->Map().begin();
        }

        inline ConstIterator End() const noexcept
        {
            return this->Map().end();
        }
    };

    template<typename _Logger>
    using MemoryTable = sharpen::InternalMemoryTable<_Logger>;
}

#endif