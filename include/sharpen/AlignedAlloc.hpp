#pragma once
#ifndef _SHARPEN_ALIGNEDALLOC_HPP
#define _SHARPEN_ALIGNEDALLOC_HPP

#include <cstring>
#include <cassert>
#include <limits>

#include <cstdint>
#include <cstddef>

namespace sharpen
{
    void *AlignedAlloc(std::size_t size,std::size_t alignment) noexcept;

    void AlignedFree(void *memblock) noexcept;

    inline void *AlignedCalloc(std::size_t count,std::size_t size,std::size_t alignment) noexcept
    {
        assert(count != 0 && size != 0);
        assert(std::numeric_limits<std::size_t>::max()/size >= count);
        void *mem = sharpen::AlignedAlloc(size*count,alignment);
        if(mem != nullptr)
        {
            std::memset(mem,0,size*count);
        }
        return mem;
    }
}

#endif