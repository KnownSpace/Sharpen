#pragma once
#ifndef _SHARPEN_FILEPOINTER_HPP
#define _SHARPEN_FILEPOINTER_HPP

#include <cstddef>
#include <cstdint>

namespace sharpen
{
    struct FilePointer
    {
        std::uint64_t offset_;
        std::uint64_t size_;
    };
}   // namespace sharpen

#endif