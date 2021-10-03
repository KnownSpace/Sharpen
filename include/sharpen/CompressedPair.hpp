#pragma once
#ifndef _SHARPEN_COMPRESSEDPAIR_HPP
#define _SHARPEN_COMPRESSEDPAIR_HPP

#include "TypeDef.hpp"
#include "TypeTraits.hpp"

namespace sharpen
{
    template<typename _T1,typename _T2,bool _T1IsEmpty,bool _T2IsEmpty>
    class InternalCompressedPair
    {
    private:
        using Self = sharpen::InternalCompressedPair<_T1,_T2,_T1IsEmpty,_T2IsEmpty>;

        _T1 first_;
        _T2 second_;
    public:
        InternalCompressedPair(_T1 first,_T2 second)
            :first_(std::move(first))
            ,second_(std::move(second))
        {}

        InternalCompressedPair(const Self &other)
            :first_(other.first_)
            ,second_(other.second_)
        {}

        InternalCompressedPair(Self &&other) noexcept
            :first_(std::move(other.first_))
            ,second_(std::move(other.second_))
        {}

        Self &operator=(const Self &other)
        {
            Self tmp(other);
            std::swap(*this,tmp);
            return *this;
        }

        Self &operator=(Self &&other) noexcept
        {
            this->first_ = std::move(other.first_);
            this->second_ = std::move(other.first_);
            return *this;
        }

        void Swap(Self &other) noexcept
        {
            if (&other != this)
            {
                std::swap(this->first_,other.first_);
                std::swap(this->second_,other.second_);
            }
        }

        inline void swap(Self &other) noexcept
        {
            this->Swap(other);
        }

        ~InternalCompressedPair() noexcept = default;

        _T1 &First() noexcept
        {
            return this->first_;
        }

        const _T1 &First() const noexcept
        {
            return this->second_;
        }

        _T2 &Second() noexcept
        {
            return this->second_;
        }

        const _T2 &Second() const noexcept
        {
            return this->second_;
        }
    };

    template<typename _T1,typename _T2>
    class InternalCompressedPair<_T1,_T2,true,true>:private _T1,private _T2
    {
    private:
        using Self  = sharpen::InternalCompressedPair<_T1,_T2,true,true>;
        using MyFirstBase = _T1;
        using MySecondBase = _T2;
    public:
        InternalCompressedPair() = default;

        InternalCompressedPair(_T1 first,_T2 second)
            :MyFirstBase(std::move(first))
            ,MySecondBase(std::move(second))
        {}

        InternalCompressedPair(const Self &other) = default;

        InternalCompressedPair(Self &&other) noexcept = default;

        Self &operator=(const Self &other) = default;

        Self &operator=(Self &&other) noexcept = default;

        ~InternalCompressedPair() noexcept = default;

        _T1 &First() noexcept
        {
            return *this;
        }

        const _T1 &First() const noexcept
        {
            return *this;
        }

        _T2 &Second() noexcept
        {
            return *this;
        }

        const _T2 &Second() const noexcept
        {
            return *this;
        }

        void Swap(Self &other) noexcept
        {
            if (&other != this)
            {
                MyFirstBase &first = *this,&otherFirst = other;
                MySecondBase &second = *this,&otherSecond = other;
                std::swap(first,otherFirst);
                std::swap(second,otherSecond);
            }
        }

        inline void swap(Self &other) noexcept
        {
            this->Swap(other);
        }
    };

    template<typename _T1,typename _T2>
    class InternalCompressedPair<_T1,_T2,true,false>:private _T1
    {
    private:
        using Self = sharpen::InternalCompressedPair<_T1,_T2,true,false>;
        using MyBase = _T1;

        _T2 second_;
    public:
        InternalCompressedPair() = default;

        InternalCompressedPair(_T1 first,_T2 second)
            :MyBase(std::move(first))
            ,second_(std::move(second))
        {}

        InternalCompressedPair(const Self &other)
            :MyBase(other)
            ,second_(other.second_)
        {}

        InternalCompressedPair(Self &&other) noexcept
            :MyBase(std::move(other))
            ,second_(std::move(other.second_))
        {}

        Self &operator=(const Self &other)
        {
            Self tmp(other);
            std::swap(*this,tmp);
            return *this;
        }

        Self &operator=(Self &&other) noexcept
        {
            if (&other != this)
            {
                MyBase::operator=(std::move(other));
                this->second_ = std::move(other.second_);
            }
            return *this;
        }

        ~InternalCompressedPair() noexcept = default;

        _T1 &First() noexcept
        {
            return *this;
        }

        const _T1 &First() const noexcept
        {
            return *this;
        }

        _T2 &Second() noexcept
        {
            return this->second_;
        }

        const _T2 &Second() const noexcept
        {
            return this->second_;
        }

        void Swap(Self &other) noexcept
        {
            if (&other != this)
            {
                std::swap(this->second_,other.second_);
                MyBase &base = *this,&otherBase = other;
                std::swap(base,otherBase);
            }
        }

        inline void swap(Self &other) noexcept
        {
            this->Swap(other);
        }
    };

    template<typename _T1,typename _T2>
    class InternalCompressedPair<_T1,_T2,false,true>:private _T2
    {
    private:
        using MyBase = _T2;
        using Self = sharpen::InternalCompressedPair<_T1,_T2,false,true>;
        
        _T1 first_;
    public:
        InternalCompressedPair() = default;

        InternalCompressedPair(_T1 first,_T2 second)
            :MyBase(std::move(second))
            ,first_(std::move(first))
        {}

        InternalCompressedPair(const Self &other)
            :MyBase(other)
            ,first_(other.first_)
        {}

        InternalCompressedPair(Self &&other) noexcept
            :MyBase(std::move(other))
            ,first_(std::move(other.first_))
        {}

        Self &operator=(const Self &other)
        {
            Self tmp(other);
            std::swap(tmp,*this);
            return *this;
        }

        Self &operator=(Self &&other) noexcept
        {
            if (&other != this)
            {
                MyBase::operator=(std::move(other));
                this->first_ = std::move(other.first_);
            }
            return *this;
        }

        ~InternalCompressedPair() noexcept = default;

        _T1 &First() noexcept
        {
            return this->first_;
        }

        const _T1 &First() const noexcept
        {
            return this->first_;
        }

        _T2 &Second() noexcept
        {
            return *this;
        }

        const _T2 &Second() const noexcept
        {
            return *this;
        }

        void Swap(Self &other) noexcept
        {
            if (&other != this)
            {
                std::swap(this->frist_,other.first_);
                MyBase &base = *this,&otherBase = other;
                std::swap(base,otherBase);
            }
        }

        inline void swap(Self &other) noexcept
        {
            this->Swap(other);
        }
    };

    template<typename _T>
    class InternalCompressedPair<_T,_T,true,true>:private _T
    {
    private:
        using MyBase = _T;
        using Self = sharpen::InternalCompressedPair<_T,_T,true,true>;

        _T second_;

    public:
        InternalCompressedPair() = default;

        InternalCompressedPair(_T first,_T second)
            :MyBase(std::move(first))
            ,second_(std::move(second))
        {}

        InternalCompressedPair(const Self &other)
            :MyBase(other)
            ,second_(other.second_)
        {}

        InternalCompressedPair(Self &&other) noexcept
            :MyBase(std::move(other))
            ,second_(std::move(other.second_))
        {}

        Self &operator=(const Self &other)
        {
            Self tmp(other);
            std::swap(*this,tmp);
            return *this;
        }

        Self &operator=(Self &&other) noexcept
        {
            if (&other != this)
            {
                MyBase::operator=(std::move(other));
                this->second_ = std::move(other.second_);
            }
            return *this;
        }

        ~InternalCompressedPair() noexcept = default;

        _T &First() noexcept
        {
            return *this;
        }

        const _T &First() const noexcept
        {
            return *this;
        }

        _T &Second() noexcept
        {
            return this->second_;
        }

        const _T &Second() const noexcept
        {
            return this->second_;
        }

        void Swap(Self &other) noexcept
        {
            if (&other != this)
            {
                std::swap(this->second_,other.second_);
                MyBase &base = *this,&otherBase = other;
                std::swap(base,otherBase);
            }
        }

        inline void swap(Self &other) noexcept
        {
            this->Swap(other);
        }
    };

    template<typename _T1,typename _T2>
    using CompressedPair = sharpen::InternalCompressedPair<_T1,_T2,sharpen::IsEmptyType<_T1>::Value,sharpen::IsEmptyType<_T2>::Value>;
}

#endif