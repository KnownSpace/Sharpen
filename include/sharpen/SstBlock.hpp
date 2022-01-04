#pragma once
#ifndef _SHARPEN_SSTBLOCK_HPP
#define _SHARPEN_SSTBLOCK_HPP

#include "TypeDef.hpp"

namespace sharpen
{
    struct SstBlock
    {
        sharpen::Uint64 offset_;
        sharpen::Uint64 size_;
    };
}

#endif