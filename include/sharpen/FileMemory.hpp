#pragma once
#ifndef _SHARPEN_FILEMEMORY_HPP
#define _SHARPEN_FILEMEMORY_HPP

#include "TypeDef.hpp"
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
        sharpen::Size size_;
#ifdef SHARPEN_IS_WIN
        sharpen::FileHandle file_;
#endif
    public:

#ifdef SHARPEN_IS_WIN
        FileMemory(sharpen::FileHandle file,void *address,sharpen::Size size);
#else
        FileMemory(void *address,sharpen::Size size) noexcept;
#endif

        FileMemory(Self &&other) noexcept;

        Self &operator=(Self &&other) noexcept;

        ~FileMemory() noexcept;

        void *Get() const noexcept;

        void Flush() const;

        void FlushSync() const;
    }; 
}

#endif