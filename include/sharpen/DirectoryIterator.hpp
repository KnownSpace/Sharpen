#pragma once
#ifndef _SHARPEN_DIRECTORYITERATOR_HPP
#define _SHARPEN_DIRECTORYITERATOR_HPP

#include "Dentry.hpp"
#include "Noncopyable.hpp"

namespace sharpen
{
    class Directory;

    class DirectoryIterator:public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::DirectoryIterator;
    
        sharpen::Directory *dir_;
        sharpen::Dentry dentry_;

        void Next();
    public:
    
        explicit DirectoryIterator(sharpen::Directory *dir) noexcept;
    
        DirectoryIterator(Self &&other) noexcept;
    
        Self &operator=(Self &&other) noexcept;
    
        ~DirectoryIterator() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        const sharpen::Dentry &operator*() const noexcept;

        const sharpen::Dentry *operator->() const noexcept;

        void operator++();

        void operator++(int);

        bool operator==(const Self &other) const noexcept;

        bool operator!=(const Self &other) const noexcept;
    };
}

#endif