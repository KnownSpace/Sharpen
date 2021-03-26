#pragma once
#ifndef _SHARPEN_MEMORYSTACK_HPP
#define _SHARPEN_MEMORYSTACK_HPP

#include "TypeDef.hpp"
#include "Noncopyable.hpp"

namespace sharpen
{
    class MemoryStack:public sharpen::Noncopyable
    {
    private:
        using Self = MemoryStack;

        void *mem_;
        sharpen::Size size_;

    public:
        MemoryStack();

        MemoryStack(void *mem,sharpen::Size size);

        MemoryStack(Self &&other) noexcept;

        ~MemoryStack() noexcept;

        void *Top() const noexcept;

        void *Bottom() const noexcept;

        sharpen::Size Size() const noexcept;

        void Release() noexcept;

        Self &operator=(Self &&other) noexcept;

        void Swap(Self &other) noexcept;

        inline void swap(Self &other) noexcept
        {
            return this->Swap(other);
        }

        static sharpen::MemoryStack AllocStack(sharpen::Size size);

        void Extend(sharpen::Size newSize);

        void ExtendNoSave(sharpen::Size newSize);

        void Clean() noexcept;

        bool Validate() const noexcept;

        operator bool() const noexcept
        {
            return this->Validate();
        }
    };
}

#endif