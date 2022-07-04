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
    
    class EmptyOptional
    {};

    constexpr sharpen::EmptyOptional EmptyOpt;

    class BadOptionException:public std::logic_error
    {
    private:
        using Base = std::logic_error;
        using Self = sharpen::BadOptionException;
    public:
        explicit BadOptionException(const char *str) noexcept
            :Base(str)
        {}

        BadOptionException(const Self &other) noexcept = default;

        BadOptionException(Self &&other) noexcept = default;

        ~BadOptionException() noexcept = default;

        Self &operator=(const Self &other) noexcept = default;

        Self &operator=(Self &&other) noexcept = default;
    };

    template<typename _T,bool _IsTrivial>
    class InternalOptional
    {
    private:
        using Self = sharpen::InternalOptional<_T,_IsTrivial>;

        bool hasValue_;
        union 
        {
            _T value_;
            sharpen::InternalOptionDummpType dummy_;
        };
    public:
        InternalOptional() noexcept
            :hasValue_(false)
            ,dummy_()
        {}

        InternalOptional(const Self &other)
            :hasValue_(false)
            ,dummy_()
        {
            if (other.hasValue_)
            {
                this->value_ = other.value_;
                this->hasValue_ = other.hasValue_;
            }
        }

        InternalOptional(Self &&other) noexcept
            :hasValue_(false)
            ,dummy_()
        {
            if (other.hasValue_)
            {
                this->value_ = other.value_;
                this->hasValue_ = true;
                other.hasValue_ = false;
            }
        }

        template<typename ..._Args,typename _Check = decltype(_T{std::declval<_Args>()...})>
        InternalOptional(_Args &&...args) SHARPEN_NOEXCEPT_IF(_T {std::declval<_Args>()...})
            :hasValue_(false)
            ,dummy_()
        {
            ::new (&this->value_) _T{std::forward<_Args>(args)...};
            this->hasValue_ = true;
        }

        InternalOptional(sharpen::EmptyOptional)
            :hasValue_(false)
            ,dummy_()
        {}

        Self &operator=(const Self &other)
        {
            if (this != std::addressof(other))
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
            if (this != std::addressof(other))
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

        Self &operator=(sharpen::EmptyOptional) noexcept
        {
            this->Reset();
            return *this;
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

        bool Exist() const noexcept
        {
            return this->hasValue_;
        }

        void Reset() noexcept
        {
            this->hasValue_ = false;
        }

        template<typename ..._Args,typename _Check = decltype(_T{std::declval<_Args>()...})>
        void Construct(_Args &&...args) SHARPEN_NOEXCEPT_IF(_T {std::declval<_Args>()...})
        {
            this->hasValue_ = true;
            ::new (&this->value_) _T{std::forward<_Args>(args)...};
        }      

        ~InternalOptional() noexcept = default;
    };

    template<typename _T>
    class InternalOptional<_T,false>
    {
    private:
        using Self = sharpen::InternalOptional<_T,false>;

        _T *launderPtr_;
        union 
        {
            _T value_;
            sharpen::InternalOptionDummpType dummy_;
        };
    public:
        InternalOptional() noexcept
            :launderPtr_(nullptr)
            ,dummy_()
        {}

        InternalOptional(const Self &other)
            :launderPtr_(nullptr)
            ,dummy_()
        {
            if (other.launderPtr_)
            {
                this->launderPtr_ = ::new(&this->value_) _T{other.Get()};
            }
        }

        InternalOptional(Self &&other) noexcept
            :launderPtr_(nullptr)
            ,dummy_()
        {
            if (other.launderPtr_)
            {
                this->launderPtr_ = ::new(&this->value_) _T{std::move(other.Get())};
            }
            other.Reset();
        }

        template<typename ..._Args,typename _Check = decltype(_T{std::declval<_Args>()...})>
        InternalOptional(_Args &&...args) SHARPEN_NOEXCEPT_IF(_T {std::declval<_Args>()...})
            :launderPtr_(nullptr)
            ,dummy_()
        {
            this->launderPtr_ = ::new (&this->value_) _T{std::forward<_Args>(args)...};
        }

        InternalOptional(sharpen::EmptyOptional)
            :launderPtr_(nullptr)
            ,dummy_()
        {}

        Self &operator=(const Self &other)
        {
            Self tmp{other};
            std::swap(*this,tmp);
            return *this;
        }

        Self &operator=(Self &&other) noexcept
        {
            if (this != std::addressof(other))
            {
                if (this->launderPtr_)
                {
                    this->Reset();
                }
                if(other.launderPtr_)
                {
                    this->launderPtr_ = ::new(&this->value_) _T{std::move(other.Get())};
                    other.Reset();
                }
            }
            return *this;
        }

        Self &operator=(sharpen::EmptyOptional) noexcept
        {
            this->Reset();
            return *this;
        }

        _T &Get()
        {
            if (this->launderPtr_)
            {
                return *this->launderPtr_;
            }
            throw sharpen::BadOptionException("this option is null");
        }

        const _T &Get() const
        {
            if (this->launderPtr_)
            {
                return *this->launderPtr_;
            }
            throw sharpen::BadOptionException("this option is null");
        }

        bool Exist() const noexcept
        {
            return this->launderPtr_;
        }

        void Reset() noexcept
        {
            _T *ptr{nullptr};
            std::swap(ptr,this->launderPtr_);
            if (ptr)
            {
                ptr->~_T();
            }
        }

        template<typename ..._Args,typename _Check = decltype(_T{std::declval<_Args>()...})>
        void Construct(_Args &&...args) SHARPEN_NOEXCEPT_IF(_T {std::declval<_Args>()...})
        {
            this->Reset();
            this->launderPtr_ = ::new (&this->value_) _T{std::forward<_Args>(args)...};
        }

        ~InternalOptional() noexcept
        {
            Reset();
        }
    };

    template<typename _T>
    using Optional = sharpen::InternalOptional<_T,std::is_trivial<_T>::value>;
}

#endif