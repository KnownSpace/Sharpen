#pragma once
#ifndef _SHARPEN_FILEPOINTER_HPP
#define _SHARPEN_FILEPOINTER_HPP

#include <cstdint>
#include <cstddef>

namespace sharpen
{
    struct FilePointer
    {
        std::uint64_t offset_;
        std::uint64_t size_;
    };
}

#endif