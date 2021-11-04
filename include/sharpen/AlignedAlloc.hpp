#pragma once
#ifndef _SHARPEN_ALIGNEDALLOC_HPP
#define _SHARPEN_ALIGNEDALLOC_HPP

#include <cstring>

#include "TypeDef.hpp"

namespace sharpen
{
    void *AlignedAlloc(sharpen::Size size,sharpen::Size alignment) noexcept;

    void AlignedFree(void *memblock) noexcept;

    inline void *AlignedCalloc(sharpen::Size count,sharpen::Size size,sharpen::Size alignment) noexcept
    {
        void *mem = sharpen::AlignedAlloc(size*count,alignment);
        if(mem != nullptr)
        {
            std::memset(mem,0,size*count);
        }
        return mem;
    }
}

#endif