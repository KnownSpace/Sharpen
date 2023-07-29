#pragma once
#ifndef _SHARPEN_DENTRY_HPP
#define _SHARPEN_DENTRY_HPP

#include "FileTypeDef.hpp"
#include <string>
#include <utility>

namespace sharpen {
    class Dentry {
    private:
        using Self = sharpen::Dentry;

        sharpen::FileEntryType type_;
        std::string name_;

    public:
        Dentry() noexcept;

        explicit Dentry(std::string name) noexcept;

        Dentry(sharpen::FileEntryType type, std::string name) noexcept;

        Dentry(const Self &other);

        Dentry(Self &&other) noexcept;

        inline Self &operator=(const Self &other) {
            if (this != std::addressof(other)) {
                Self tmp{other};
                std::swap(tmp, *this);
            }
            return *this;
        }

        Self &operator=(Self &&other) noexcept;

        ~Dentry() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        inline sharpen::FileEntryType GetType() const noexcept {
            return this->type_;
        }

        inline void SetType(sharpen::FileEntryType type) noexcept {
            this->type_ = type;
        }

        inline std::string &Name() noexcept {
            return this->name_;
        }

        inline const std::string &Name() const noexcept {
            return this->name_;
        }

        bool Valid() const noexcept;

        bool operator==(const Self &other) const noexcept;

        bool operator!=(const Self &other) const noexcept;
    };
}   // namespace sharpen

#endif