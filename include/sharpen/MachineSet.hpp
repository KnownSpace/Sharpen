#pragma once
#ifndef _SHARPEN_MACHINESET_HPP
#define _SHARPEN_MACHINESET_HPP

#include <cstdint>
#include <cstddef>
#include <map>

#include "ByteBuffer.hpp"
#include "Optional.hpp"

namespace sharpen
{
    class MachineSet
    {
    private:
        using Self = sharpen::MachineSet;
        using Map = std::map<std::uint64_t,sharpen::ByteBuffer>;
        using Iterator = typename Map::iterator;
        using ConstIterator = typename Map::const_iterator;
        
        Map map_;
    public:
    
        MachineSet() = default;
    
        MachineSet(const Self &other) = default;
    
        MachineSet(Self &&other) noexcept = default;
    
        inline Self &operator=(const Self &other)
        {
            if(this != std::addressof(other))
            {
                Self tmp{other};
                std::swap(tmp,*this);
            }
            return *this;
        }
    
        inline Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->map_ = std::move(other.map_);
            }
            return *this;
        }
    
        ~MachineSet() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline Iterator Begin() noexcept
        {
            return this->map_.begin();
        }
        
        inline ConstIterator Begin() const noexcept
        {
            return this->map_.begin();
        }
        
        inline Iterator End() noexcept
        {
            return this->map_.end();
        }
        
        inline ConstIterator End() const noexcept
        {
            return this->map_.end();
        }

        inline Iterator Find(std::uint64_t actorId) noexcept
        {
            auto ite = this->map_.find(actorId);
            return ite;
        }

        inline ConstIterator Find(std::uint64_t actorId) const noexcept
        {
            auto ite = this->map_.find(actorId);
            return ite;
        }

        void Insert(std::uint64_t actorId,sharpen::ByteBuffer data);

        void Remove(std::uint64_t actorId) noexcept;

        std::size_t GetSize() const noexcept;

        bool Empty() const noexcept;

        void Clear() const noexcept;
    };
}

#endif