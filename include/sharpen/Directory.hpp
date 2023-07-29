#pragma once
#ifndef _SHARPEN_DIRECTORY_HPP
#define _SHARPEN_DIRECTORY_HPP

#include "Dentry.hpp"
#include "DirectoryIterator.hpp"
#include "FileTypeDef.hpp"
#include "Noncopyable.hpp"
#include "SystemMacro.hpp"
#include <string>
#include <utility>

namespace sharpen {
    class Directory : public sharpen::Noncopyable {
    private:
        using Self = sharpen::Directory;
        using Iterator = sharpen::DirectoryIterator;

        std::string name_;
#ifdef SHARPEN_IS_WIN
        mutable sharpen::FileHandle handle_;
#else
        mutable void *handle_;
#endif

        static bool CheckName(const std::string &name) noexcept;

        void Close() noexcept;

        sharpen::Dentry InternalGetNextEntry() const;

    public:
        explicit Directory(std::string name);

        Directory(Self &&other) noexcept;

        Self &operator=(Self &&other) noexcept;

        ~Directory() noexcept;

        inline const Self &Const() const noexcept {
            return *this;
        }

        bool Exist() const;

        inline Iterator Begin() noexcept {
            return sharpen::DirectoryIterator{this};
        }

        inline Iterator End() noexcept {
            return sharpen::DirectoryIterator{nullptr};
        }

        sharpen::Dentry GetNextEntry() const;

        sharpen::RmdirResult Remove();

        sharpen::RmdirResult RemoveAll();

        inline const std::string &Path() const noexcept {
            return this->name_;
        }

        sharpen::MkdirResult Create();
    };
}   // namespace sharpen

#endif