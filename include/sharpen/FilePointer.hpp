#pragma once
#ifndef _SHARPEN_FILEPOINTER_HPP
#define _SHARPEN_FILEPOINTER_HPP

#include "TypeDef.hpp"

namespace sharpen
{
    struct FilePointer
    {
        sharpen::Uint64 offset_;
        sharpen::Uint64 size_;
    };
}

#endif