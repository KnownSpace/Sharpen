#pragma once
#ifndef _SHARPEN_ALIGNEDALLOC_HPP
#define _SHARPEN_ALIGNEDALLOC_HPP

#include <cstring>
#include <cassert>
#include <limits>
#include <cstdint>
#include <cstddef>
#include <stdexcept>

namespace sharpen
{
    void *AlignedAlloc(std::size_t size,std::size_t alignment) noexcept;

    void AlignedFree(void *memblock) noexcept;

    inline void *AlignedCalloc(std::size_t count,std::size_t size,std::size_t alignment) noexcept
    {
        assert(count != 0 && size != 0);
        assert((std::numeric_limits<std::size_t>::max)()/size >= count);
        void *mem = sharpen::AlignedAlloc(size*count,alignment);
        if(mem != nullptr)
        {
            std::memset(mem,0,size*count);
        }
        return mem;
    }

    inline void *AlignedAlocPages(std::size_t pageCount) noexcept
    {
        constexpr std::size_t pageSize{4096};
        return sharpen::AlignedCalloc(pageCount*pageSize,sizeof(char),pageSize);
    }

    class AlignedPages
    {
    private:
        using Self = AlignedPages;
    
        char *mem_;
    public:
    
        explicit AlignedPages(std::size_t pageCount)
            :mem_(reinterpret_cast<char*>(sharpen::AlignedAlocPages(pageCount)))
        {
            if(!mem_)
            {
                throw std::bad_alloc{};
            }
        }
    
        AlignedPages(Self &&other) noexcept
            :mem_(other.mem_)
        {
            other.mem_ = nullptr;
        }
    
        inline Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->mem_ = other.mem_;
                other.mem_ = nullptr;
            }
            return *this;
        }
    
        ~AlignedPages() noexcept
        {
            if(this->mem_)
            {
                sharpen::AlignedFree(this->mem_);
            }
        }
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline char *Data() const noexcept
        {
            assert(this->mem_);
            return this->mem_;
        }

        inline char &Get(std::size_t index) const noexcept
        {
            assert(this->mem_);
            return this->mem_[index];
        }

        operator char*() const noexcept
        {
            return this->Data();
        }

        inline char &operator*() const noexcept
        {
            return this->Get(0);
        }

        inline char &operator[](std::size_t index) const noexcept
        {
            return this->Get(index);
        }
    };
}

#endif