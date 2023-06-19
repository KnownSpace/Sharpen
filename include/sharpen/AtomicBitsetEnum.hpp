#pragma once
#ifndef _SHARPEN_ATOMICBITSETENUM_HPP
#define _SHARPEN_ATOMICBITSETENUM_HPP

#include "TypeTraits.hpp"
#include <atomic>

namespace sharpen {
    template<typename _Enum, _Enum _Default = static_cast<_Enum>(0)>
    class AtomicBitsetEnum {
    private:
        using Self = sharpen::AtomicBitsetEnum<_Enum, _Default>;
        using IntType = sharpen::UintType<sizeof(_Enum)>;
        using AtomicType = std::atomic<IntType>;

        AtomicType enum_;

    public:
        AtomicBitsetEnum() noexcept
            : enum_(static_cast<IntType>(_Default)) {
        }

        explicit AtomicBitsetEnum(_Enum bit) noexcept
            : enum_(static_cast<IntType>(bit)) {
        }

        AtomicBitsetEnum(const Self &other) noexcept
            : enum_(other.enum_.load()) {
        }

        AtomicBitsetEnum(Self &&other) noexcept
            : enum_(other.enum_.exchange(static_cast<IntType>(_Default))) {
        }

        inline Self &operator=(const Self &other) noexcept {
            if (this != std::addressof(other)) {
                Self tmp{other};
                std::swap(tmp, *this);
            }
            return *this;
        }

        inline Self &operator=(Self &&other) noexcept  {
            if(this != std::addressof(other)) {
                this->enum_ = other.enum_.exchange(static_cast<IntType>(_Default));
            }
            return *this;
        }

        ~AtomicBitsetEnum() noexcept = default;

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
            return Self{static_cast<_Enum>(this->enum_.exchange(static_cast<IntType>(_Default)))};
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