#pragma once
#ifndef _SHARPEN_ALIGNEDALLOC_HPP
#define _SHARPEN_ALIGNEDALLOC_HPP

#include <cstring>
#include <cassert>
#include <limits>

#include "TypeDef.hpp"

namespace sharpen
{
    void *AlignedAlloc(sharpen::Size size,sharpen::Size alignment) noexcept;

    void AlignedFree(void *memblock) noexcept;

    inline void *AlignedCalloc(sharpen::Size count,sharpen::Size size,sharpen::Size alignment) noexcept
    {
        assert(count != 0 && size != 0);
        assert(std::numeric_limits<sharpen::Size>::max()/size >= count);
        void *mem = sharpen::AlignedAlloc(size*count,alignment);
        if(mem != nullptr)
        {
            std::memset(mem,0,size*count);
        }
        return mem;
    }
}

#endif