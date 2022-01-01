#pragma once
#ifndef _SHARPEN_MEMORYTABLE_HPP
#define _SHARPEN_MEMORYTABLE_HPP

#include "ByteBuffer.hpp"
#include "MemoryTableConcepts.hpp"
#include "CompressedPair.hpp"

namespace sharpen
{
    template<typename _Map,typename _Logger>
    using MemoryTableRequire = sharpen::IsMemoryTableLogger<_Logger,_Map>;

    template<typename _Map,typename _Logger,typename _Check = void>
    class InternalMemoryTable;

    template<typename _Map,typename _Logger>
    class InternalMemoryTable<_Map,_Logger,sharpen::EnableIf<sharpen::MemoryTableRequire<_Map,_Logger>::Value>>:public sharpen::Noncopyable
    {
    private:
        using Self = InternalMemoryTable<_Map,_Logger>;
        using MapType = _Map;
        using ConstIterator = typename MapType::const_iterator;
        using Pair = sharpen::CompressedPair<MapType,_Logger>;

        Pair pair_;
    public:
        template<typename ..._Args,typename _Check = decltype(_Logger{std::declval<_Args>()...})>
        explicit InternalMemoryTable(_Args &&...args)
            :pair_(MapType{},_Logger{std::forward<_Args>(args)...})
        {}
    
        InternalMemoryTable(Self &&other) noexcept
            :pair_(std::move(other.pair_))
        {}
    
        Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->pair_ = std::move(other.pair_);
            }
            return *this;
        }

        MapType &Map() noexcept
        {
            return this->pair_.First();
        }

        const MapType &Map() const noexcept
        {
            return this->pair_.First();
        }

         _Logger &Logger() noexcept
        {
            return this->pair_.Second();
        }

        const _Logger &Logger() const noexcept
        {
            return this->pair_.Second();
        }

        void Put(const sharpen::ByteBuffer &key,sharpen::ByteBuffer value)
        {
            this->Logger().LogPut(key,value);
            this->Map()[key] = std::move(value);
        }

        void Delete(const sharpen::ByteBuffer &key)
        {
            this->Logger().LogDelete(key);
            this->Map().erase(key);
        }

        const sharpen::ByteBuffer &Get(const sharpen::ByteBuffer &key) const
        {
            return this->Map().at(key); 
        }

        const sharpen::ByteBuffer &operator[](const sharpen::ByteBuffer &key) const
        {
            return this->Get(key);
        }

        sharpen::Size GetSize() const noexcept
        {
            return this->Map().size();
        }

        bool Empty() const noexcept
        {
            return this->Map().empty();
        }

        bool Exist(const sharpen::ByteBuffer &key) const noexcept
        {
            return this->Map().find(key) != this->Map().end();
        }

        void Clear() const
        {
            this->Map().clear();
            this->Logger().ClearLogs();
        }

        void RestoreFromLogger()
        {
            this->Logger().Restore(this->Map());
        }

        ConstIterator Begin() const noexcept
        {
            return this->Map().begin();
        }

        ConstIterator End() const noexcept
        {
            return this->Map().end();
        }
    
        ~InternalMemoryTable() noexcept = default;
    };

    template<template<class,class,class ...> class _Map,typename _Logger>
    using MemoryTable = sharpen::InternalMemoryTable<_Map<sharpen::ByteBuffer,sharpen::ByteBuffer>,_Logger>;   
}

#endif