#pragma once
#ifndef _SHARPEN_ALIGNEDALLOC_HPP
#define _SHARPEN_ALIGNEDALLOC_HPP

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace sharpen {
    extern void *AlignedAlloc(std::size_t size, std::size_t alignment) noexcept;

    extern void AlignedFree(void *memblock) noexcept;

    inline void *AlignedCalloc(std::size_t count,
                               std::size_t size,
                               std::size_t alignment) noexcept {
        assert(count != 0 && size != 0);
        assert((std::numeric_limits<std::size_t>::max)() / size >= count);
        void *mem = sharpen::AlignedAlloc(size * count, alignment);
        if (mem != nullptr) {
            std::memset(mem, 0, size * count);
        }
        return mem;
    }

    inline void *AlignedAllocPages(std::size_t pageCount) noexcept {
        constexpr std::size_t pageSize{4096};
        return sharpen::AlignedCalloc(pageCount * pageSize, sizeof(char), pageSize);
    }
}   // namespace sharpen

#endif