#pragma once
#ifndef _SHARPEN_BITSETENUM_HPP
#define _SHARPEN_BITSETENUM_HPP

#include "TypeTraits.hpp"

namespace sharpen {
    template<typename _Enum, _Enum _Default = static_cast<_Enum>(0)>
    class BitsetEnum {
    private:
        using Self = sharpen::BitsetEnum<_Enum, _Default>;
        using IntType = sharpen::UintType<sizeof(_Enum)>;

        IntType enum_;

    public:
        BitsetEnum() noexcept
            : enum_(static_cast<IntType>(_Default)) {
        }

        explicit BitsetEnum(_Enum bit) noexcept
            : enum_(static_cast<IntType>(bit)) {
        }

        BitsetEnum(Self &&other) noexcept
            : enum_(other.enum_) {
            other.enum_ = static_cast<IntType>(_Default);
        }

        inline Self &operator=(const Self &other) noexcept {
            if (this != std::addressof(other)) {
                Self tmp{other};
                std::swap(tmp, *this);
            }
            return *this;
        }

        inline Self &operator=(Self &&other) noexcept {
            if (this != std::addressof(other)) {
                this->enum_ = other.enum_;
                other.enum_ = static_cast<IntType>(_Default);
            }
            return *this;
        }

        ~BitsetEnum() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        inline bool IsSet(_Enum bit) const noexcept {
            return this->enum_ & static_cast<IntType>(bit);
        }

        inline void Set(_Enum bit) noexcept {
            this->enum_ |= static_cast<IntType>(bit);
        }

        inline void Unset(_Enum bit) noexcept {
            this->enum_ &= ~static_cast<IntType>(bit);
        }

        inline void Clear() noexcept {
            this->enum_ = static_cast<IntType>(_Default);
        }

        inline Self Take() noexcept {
            Self tmp{static_cast<IntType>(this->enum_)};
            this->Clear();
            return tmp;
        }

        inline bool IsDefault() const noexcept {
            return this->enum_ == static_cast<IntType>(_Default);
        }

        inline bool operator==(const Self &other) const noexcept
        {
            return this->enum_ == other.enum_;
        }
        
        inline bool operator!=(const Self &other) const noexcept
        {
            return this->enum_ != other.enum_;
        }
    };
}   // namespace sharpen

#endif