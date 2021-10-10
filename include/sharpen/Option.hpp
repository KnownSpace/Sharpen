#pragma once
#ifndef _SHARPEN_OPTION_HPP
#define _SHARPEN_OPTION_HPP

#include <type_traits>
#include <new>
#include <cstring>
#include <stdexcept>

#include "NoexceptIf.hpp"

namespace sharpen
{
    struct InternalOptionDummpType
    {
        constexpr InternalOptionDummpType() noexcept
        {}
    };
    
    class NullOption
    {};

    constexpr sharpen::NullOption NullOpt;

    class BadOptionException:public std::logic_error
    {
    private:
        using Base = std::logic_error;
    public:
        explicit BadOptionException(const char *str) noexcept
            :Base(str)
        {}
    };

    template<typename _T,bool _IsTrivial>
    class InternalOption
    {
    private:
        using Self = sharpen::InternalOption<_T,_IsTrivial>;

        bool hasValue_;
        union 
        {
            _T value_;
            sharpen::InternalOptionDummpType dummp_;
        };
    public:
        InternalOption() noexcept
            :hasValue_(false)
            ,dummp_()
        {}

        InternalOption(const Self &other)
            :hasValue_(other.hasValue_)
            ,dummp_()
        {
            if (this->hasValue_)
            {
                this->value_ = other.value_;
            }
        }

        InternalOption(Self &&other) noexcept
            :hasValue_(other.hasValue_)
            ,dummp_()
        {
            if (this->hasValue_)
            {
                this->value_ = other.value_;
                other.hasValue_ = false;
            }
        }

        template<typename ..._Args,typename _Check = decltype(_T{std::declval<_Args>()...})>
        InternalOption(_Args &&...args) SHARPEN_NOEXCEPT_IF(_T {std::declval<_Args>()...})
            :hasValue_(true)
            ,dummp_()
        {
            ::new (&this->value_) _T{std::forward<_Args>(args)...};
        }

        InternalOption(sharpen::NullOption)
            :hasValue_(false)
            ,dummp_()
        {}

        Self &operator=(const Self &other)
        {
            if (this != &other)
            {
                this->hasValue_ = other.hasValue_;
                if (this->hasValue_)
                {
                    this->value_ = other.value_;
                }
            }
            return *this;
        }

        Self &operator=(Self &&other) noexcept
        {
            if (this != &other)
            {
                std::swap(this->hasValue_,other.hasValue_);
                other.hasValue_ = false;
                if (this->hasValue_)
                {
                    this->value_ = std::move(other.value_);
                }
            }
            return *this;
        }

        Self &operator=(sharpen::NullOption) noexcept
        {
            this->Reset();
            return *this;
        }

        void Swap(Self &other) noexcept
        {
            if (this == &other)
            {
                return;
            }
            if (this->hasValue_ && other.hasValue_)
            {
                std::swap(this->value_,this->value_);
                return;
            }
            if (this->hasValue_ && !other.hasValue_)
            {
                other.value_ = this->value_;
                std::swap(this->hasValue_,other.hasValue_);
                return;
            }
            this->value_ = other.value_;
            std::swap(this->hasValue_,other.hasValue_);
        }

        inline void swap(Self &other) noexcept
        {
            this->Swap(other);
        }

        _T &Get()
        {
            if (this->hasValue_)
            {
                return this->value_;
            }
            throw sharpen::BadOptionException("this option is null");
        }

        const _T &Get() const
        {
            if (this->hasValue_)
            {
                return this->value_;
            }
            throw sharpen::BadOptionException("this option is null");
        }

        bool HasValue() const noexcept
        {
            return this->hasValue_;
        }

        operator bool() const noexcept
        {
            return this->HasValue();
        }

        void Reset() noexcept
        {
            this->hasValue_ = false;
        }      

        ~InternalOption() noexcept = default;
    };

    template<typename _T>
    class InternalOption<_T,false>
    {
    private:
        using Self = sharpen::InternalOption<_T,false>;

        bool hasValue_;
        union 
        {
            _T value_;
            sharpen::InternalOptionDummpType dummp_;
        };
    public:
        InternalOption() noexcept
            :hasValue_(false)
            ,dummp_()
        {}

        InternalOption(const Self &other)
            :hasValue_(other.hasValue_)
            ,dummp_()
        {
            if (this->hasValue_)
            {
                this->value_ = other.value_;
            }
        }

        InternalOption(Self &&other) noexcept
            :hasValue_(other.hasValue_)
            ,dummp_()
        {
            if (this->hasValue_)
            {
                this->value_ = std::move(other.value_);
            }
            other.Reset();
        }

        template<typename ..._Args,typename _Check = decltype(_T{std::declval<_Args>()...})>
        InternalOption(_Args &&...args) SHARPEN_NOEXCEPT_IF(_T {std::declval<_Args>()...})
            :hasValue_(true)
            ,dummp_()
        {
            ::new (&this->value_) _T{std::forward<_Args>(args)...};
        }

        InternalOption(sharpen::NullOption)
            :hasValue_(false)
            ,dummp_()
        {}

        Self &operator=(const Self &other)
        {
            Self tmp(other);
            std::swap(*this,other);
            return *this;
        }

        Self &operator=(Self &&other) noexcept
        {
            if (this != &other)
            {
                if (this->hasValue_)
                {
                    this->Reset();
                }
                this->hasValue_ = true;
                this->value_ = std::move(other.value_);
                other.Reset();
            }
            return *this;
        }

        Self &operator=(sharpen::NullOption) noexcept
        {
            this->Reset();
        }

        void Swap(Self &other) noexcept
        {
            if (this == &other)
            {
                return;
            }
            if (this->hasValue_ && other.hasValue_)
            {
                std::swap(this->value_,other. value_);
                return;
            }
            if (this->hasValue_ && !other.hasValue_)
            {
                std::swap(this->hasValue_,other.hasValue_);
                ::new (&other.value_) _T(std::move(this->value_));
                this->Reset();
                return;
            }
            std::swap(this->hasValue_,other.hasValue_);
            ::new (&this->value_) _T(std::move(other.value_));
            other.Reset();
            return;
        }

        inline void swap(Self &other) noexcept
        {
            this->Swap(other);
        }

        _T &Get()
        {
            if (this->hasValue_)
            {
                return this->value_;
            }
            throw sharpen::BadOptionException("this option is null");
        }

        const _T &Get() const
        {
            if (this->hasValue_)
            {
                return this->value_;
            }
            throw sharpen::BadOptionException("this option is null");
        }

        bool HasValue() const noexcept
        {
            return this->hasValue_;
        }

        operator bool() const noexcept
        {
            return this->HasValue();
        }

        void Reset() noexcept
        {
            bool has = false;
            std::swap(has,this->hasValue_);
            if (has)
            {
                this->value_.~_T();
            }
        }

        ~InternalOption() noexcept
        {
            Reset();
        }
    };

    template<typename _T>
    using Option = sharpen::InternalOption<_T,std::is_trivial<_T>::value>;
}

#endif