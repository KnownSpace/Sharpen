#pragma once
#ifndef _SHARPEN_MEMORYSTACK_HPP
#define _SHARPEN_MEMORYSTACK_HPP

#include <cstdint>
#include <cstddef>
#include "Noncopyable.hpp"

namespace sharpen
{
    class MemoryStack:public sharpen::Noncopyable
    {
    private:
        using Self = MemoryStack;

        void *mem_;
        std::size_t size_;

    public:
        MemoryStack();

        MemoryStack(void *mem,std::size_t size);

        MemoryStack(Self &&other) noexcept;

        ~MemoryStack() noexcept;

        void *Top() const noexcept;

        void *Bottom() const noexcept;

        std::size_t Size() const noexcept;

        void Release() noexcept;

        Self &operator=(Self &&other) noexcept;

        void Swap(Self &other) noexcept;

        inline void swap(Self &other) noexcept
        {
            return this->Swap(other);
        }

        static sharpen::MemoryStack AllocStack(std::size_t size);

        void Extend(std::size_t newSize);

        void ExtendNoSave(std::size_t newSize);

        void Clean() noexcept;

        bool Validate() const noexcept;

        operator bool() const noexcept
        {
            return this->Validate();
        }
    };
}

#endif