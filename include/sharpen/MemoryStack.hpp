#pragma once
#ifndef _SHARPEN_MEMORYSTACK_HPP
#define _SHARPEN_MEMORYSTACK_HPP

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "Noncopyable.hpp"

namespace sharpen
{
    class MemoryStack : public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::MemoryStack;

        void *mem_;
        std::size_t size_;

        inline static void *Alloc(std::size_t size) noexcept
        {
            return std::calloc(size, sizeof(char));
        }

        inline static void Free(void *mem) noexcept
        {
            return std::free(mem);
        }

    public:
        MemoryStack() noexcept;

        MemoryStack(void *mem, std::size_t size) noexcept;

        MemoryStack(Self &&other) noexcept;

        ~MemoryStack() noexcept;

        static sharpen::MemoryStack AllocStack(std::size_t size);

        void *Top() const noexcept;

        inline void *Bottom() const noexcept
        {
            return this->mem_;
        }

        inline std::size_t GetSize() const noexcept
        {
            return this->size_;
        }

        void Release() noexcept;

        Self &operator=(Self &&other) noexcept;

        void Extend(std::size_t newSize);

        void ExtendNoSave(std::size_t newSize);

        inline void Clean() noexcept
        {
            if (this->mem_)
            {
                std::memset(this->mem_, 0, this->size_);
            }
        }

        inline bool Validate() const noexcept
        {
            return this->mem_;
        }

        inline operator bool() const noexcept
        {
            return this->Validate();
        }
    };
}   // namespace sharpen

#endif