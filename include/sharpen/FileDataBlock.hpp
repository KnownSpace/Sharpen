#pragma once
#ifndef _SHARPEN_FILEDATABLOCK_HPP
#define _SHARPEN_FILEDATABLOCK_HPP

#include "TypeDef.hpp"

namespace sharpen
{
    struct FileDataBlock
    {
        sharpen::Uint64 offset_;
        sharpen::Uint64 size_;
    };
}

#endif