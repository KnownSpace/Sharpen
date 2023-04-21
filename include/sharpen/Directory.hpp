#pragma once
#ifndef _SHARPEN_DIRECTORY_HPP
#define _SHARPEN_DIRECTORY_HPP

#include <string>
#include <utility>

#include "FileTypeDef.hpp"
#include "Noncopyable.hpp"

namespace sharpen
{
    class Directory : public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::Directory;

        std::string name_;
        sharpen::FileHandle handle_;

    public:
        explicit Directory(std::string name);

        Directory(Self &&other) noexcept;

        Self &operator=(Self &&other) noexcept;

        ~Directory() noexcept = default;

        inline const Self &Const() const noexcept
        {
            return *this;
        }
    };
}   // namespace sharpen

#endif