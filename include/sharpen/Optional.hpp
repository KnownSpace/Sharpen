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

    class BadOptionError:public std::logic_error
    {
    private:
        using Base = std::logic_error;
        using Self = sharpen::BadOptionError;
    public:
        explicit BadOptionError(const char *str) noexcept
            :Base(str)
        {}

        BadOptionError(const Self &other) noexcept = default;

        BadOptionError(Self &&other) noexcept = default;

        ~BadOptionError() noexcept = default;

        Self &operator=(const Self &other) noexcept = default;

        Self &operator=(Self &&other) noexcept = default;
    };

    template<typename _T,bool _IsTrivial>
    class InternalOptional
    {
    private:
        using Self = sharpen::InternalOptional<_T,_IsTrivial>;

        bool existValue_;
        union
        {
            _T value_;
            sharpen::InternalOptionDummpType dummy_;
        };
    public:
        InternalOptional() noexcept
            :existValue_(false)
            ,dummy_()
        {}

        InternalOptional(const Self &other)
            :existValue_(false)
            ,dummy_()
        {
            if(other.existValue_)
            {
                this->value_ = other.value_;
                this->existValue_ = other.existValue_;
            }
        }

        InternalOptional(Self &&other) noexcept
            :existValue_(false)
            ,dummy_()
        {
            if(other.existValue_)
            {
                this->value_ = other.value_;
                this->existValue_ = true;
                other.existValue_ = false;
            }
        }

        template<typename ..._Args,typename _Check = decltype(_T{std::declval<_Args>()...})>
        InternalOptional(_Args &&...args) SHARPEN_NOEXCEPT_IF(_T{std::declval<_Args>()...})
            :existValue_(false)
            ,dummy_()
        {
            ::new (&this->value_) _T{std::forward<_Args>(args)...};
            this->existValue_ = true;
        }

        InternalOptional(sharpen::EmptyOptional)
            :existValue_(false)
            ,dummy_()
        {}

        inline Self &operator=(const Self &other)
        {
            if(this != std::addressof(other))
            {
                this->existValue_ = other.existValue_;
                if(this->existValue_)
                {
                    this->value_ = other.value_;
                }
            }
            return *this;
        }

        inline Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->existValue_ = false;
                std::swap(this->existValue_,other.existValue_);
                if(this->existValue_)
                {
                    this->value_ = std::move(other.value_);
                }
            }
            return *this;
        }

        inline Self &operator=(sharpen::EmptyOptional) noexcept
        {
            this->Reset();
            return *this;
        }

        inline _T &Get()
        {
            if(this->existValue_)
            {
                return this->value_;
            }
            throw sharpen::BadOptionError("this option is null");
        }

        inline const _T &Get() const
        {
            if(this->existValue_)
            {
                return this->value_;
            }
            throw sharpen::BadOptionError("this option is null");
        }

        inline bool Exist() const noexcept
        {
            return this->existValue_;
        }

        inline void Reset() noexcept
        {
            this->existValue_ = false;
        }

        template<typename ..._Args,typename _Check = decltype(_T{std::declval<_Args>()...})>
        inline void Construct(_Args &&...args) SHARPEN_NOEXCEPT_IF(_T{std::declval<_Args>()...})
        {
            this->existValue_ = true;
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
            if(other.launderPtr_)
            {
                this->launderPtr_ = ::new(&this->value_) _T{other.Get()};
            }
        }

        InternalOptional(Self &&other) noexcept
            :launderPtr_(nullptr)
            ,dummy_()
        {
            if(other.launderPtr_)
            {
                this->launderPtr_ = ::new(&this->value_) _T{std::move(other.Get())};
            }
            other.Reset();
        }

        template<typename ..._Args,typename _Check = decltype(_T{std::declval<_Args>()...})>
        InternalOptional(_Args &&...args) SHARPEN_NOEXCEPT_IF(_T{std::declval<_Args>()...})
            :launderPtr_(nullptr)
            ,dummy_()
        {
            this->launderPtr_ = ::new (&this->value_) _T{std::forward<_Args>(args)...};
        }

        InternalOptional(sharpen::EmptyOptional)
            :launderPtr_(nullptr)
            ,dummy_()
        {}

        inline Self &operator=(const Self &other)
        {
            if(this != std::addressof(other))
            {
                Self tmp{other};
                std::swap(*this,tmp);
            }
            return *this;
        }

        inline Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                if(this->launderPtr_)
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

        inline Self &operator=(sharpen::EmptyOptional) noexcept
        {
            this->Reset();
            return *this;
        }

        inline _T &Get()
        {
            if(this->launderPtr_)
            {
                return *this->launderPtr_;
            }
            throw sharpen::BadOptionError("this option is null");
        }

        inline const _T &Get() const
        {
            if(this->launderPtr_)
            {
                return *this->launderPtr_;
            }
            throw sharpen::BadOptionError("this option is null");
        }

        inline bool Exist() const noexcept
        {
            return this->launderPtr_;
        }

        inline void Reset() noexcept
        {
            _T *ptr{nullptr};
            std::swap(ptr,this->launderPtr_);
            if(ptr)
            {
                ptr->~_T();
            }
        }

        template<typename ..._Args,typename _Check = decltype(_T{std::declval<_Args>()...})>
        inline void Construct(_Args &&...args) SHARPEN_NOEXCEPT_IF(_T{std::declval<_Args>()...})
        {
            this->Reset();
            this->launderPtr_ = ::new (&this->value_) _T{std::forward<_Args>(args)...};
        }

        ~InternalOptional() noexcept
        {
            this->Reset();
        }
    };

    template<typename _T>
    class Optional:public sharpen::InternalOptional<_T,std::is_trivial<_T>::value>
    {
    private:
        using Self = sharpen::Optional<_T>;
        using Base = sharpen::InternalOptional<_T,std::is_trivial<_T>::value>;

    public:
    
        Optional() noexcept
            :Base()
        {}
    
        Optional(const Self &other)
            :Base(other)
        {}
    
        Optional(Self &&other) noexcept
            :Base(std::move(other))
        {}

        template<typename ..._Args,typename _Check = decltype(_T{std::declval<_Args>()...})>
        Optional(_Args &&...args) SHARPEN_NOEXCEPT_IF(_T{std::declval<_Args>()...})
            :Base(std::forward<_Args>(args)...)
        {}

        Optional(sharpen::EmptyOptional empty) noexcept
            :Base(empty)
        {}
    
        inline Self &operator=(const Self &other)
        {
            Base::operator=(other);
            return *this;
        }
    
        inline Self &operator=(Self &&other) noexcept
        {
            Base::operator=(std::move(other));
            return *this;
        }

        inline Self &operator=(sharpen::EmptyOptional empty) noexcept
        {
            Base::operator=(empty);
            return *this;
        }
    
        ~Optional() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }
    };
}

#endif