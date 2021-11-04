#pragma once
#ifndef _SHARPEN_ALIGNEDALLOC_HPP
#define _SHARPEN_ALIGNEDALLOC_HPP

#include "TypeDef.hpp"

namespace sharpen
{
    void *AlignedAlloc(sharpen::Size size,sharpen::Size alignment) noexcept;

    void AlignedFree(void *memblock) noexcept;
}

#endif