#pragma once
#ifndef _SHARPEN_FILEMEMORY_HPP
#define _SHARPEN_FILEMEMORY_HPP

#include <cstdint>
#include <cstddef>
#include "Noncopyable.hpp"
#include "FileTypeDef.hpp"
#include "SystemMacro.hpp"

namespace sharpen
{
    class FileMemory:public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::FileMemory;

        void *address_;
        std::size_t size_;
#ifdef SHARPEN_IS_WIN
        sharpen::FileHandle file_;
#endif
    public:

#ifdef SHARPEN_IS_WIN
        FileMemory(sharpen::FileHandle file,void *address,std::size_t size) noexcept;
#else
        FileMemory(void *address,std::size_t size) noexcept;
#endif

        FileMemory(Self &&other) noexcept;

        Self &operator=(Self &&other) noexcept;

        ~FileMemory() noexcept;

        void *Get() const noexcept;

        void Flush() const;

        void FlushAndWait() const;
    }; 
}

#endif