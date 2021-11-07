#pragma once
#ifndef _SHARPEN_DUMMYTYPE_HPP
#define _SHARPEN_DUMMYTYPE_HPP

#include <cassert>
#include <utility>
#include <new>

#include "TypeTraits.hpp"
#include "NoexceptIf.hpp"

namespace sharpen
{
    template<sharpen::Size _Index,template<sharpen::Size> class _TIterator>
    struct InternalDummyTypeDtor
    {
        static void Release(sharpen::Size typeIndex,char *p) noexcept
        {
            using _T = _TIterator<_Index>;
            if (_Index == typeIndex)
            {
                _T *obj = reinterpret_cast<_T*>(p);
                obj->~_T();
                return;
            }
            sharpen::InternalDummyTypeDtor<_Index - 1,_TIterator>::Release(typeIndex,p);
        }
    };
    
    template<template<sharpen::Size> class _TIterator>
    struct InternalDummyTypeDtor<0,_TIterator>
    {
        static void Release(sharpen::Size typeIndex,char *p) noexcept
        {
            using _T = _TIterator<0>;
            if (0 == typeIndex)
            {
                _T *obj = reinterpret_cast<_T*>(p);
                obj->~_T();
            }
        }
    };

    template<sharpen::Size _Index,template<sharpen::Size> class _TIterator>
    struct InternalDummyTypeCopy
    {
        static void CopyAssign(sharpen::Size typeIndex,char *dst,const char *src)
        {
            using _T = _TIterator<_Index>;
            if (_Index == typeIndex)
            {
                _T *dstObj = reinterpret_cast<_T*>(dst);
                const _T *srcObj = reinterpret_cast<const _T*>(src);
                *dstObj = *srcObj;
                return;
            }
            sharpen::InternalDummyTypeCopy<_Index - 1,_TIterator>::CopyAssign(typeIndex,dst,src);
        }

        static void CopyConstruct(sharpen::Size typeIndex,char *dst,const char *src)
        {
            using _T = _TIterator<_Index>;
            if (_Index == typeIndex)
            {
                const _T *srcObj = reinterpret_cast<const _T*>(src);
                new (dst) _T{*srcObj};
                return;
            }
            sharpen::InternalDummyTypeCopy<_Index - 1,_TIterator>::CopyConstruct(typeIndex,dst,src);
        }
    };
    
    template<template<sharpen::Size> class _TIterator>
    struct InternalDummyTypeCopy<0,_TIterator>
    {
        static void CopyAssign(sharpen::Size typeIndex,char *dst,const char *src)
        {
            using _T = _TIterator<0>;
            if (0 == typeIndex)
            {
                _T *dstObj = reinterpret_cast<_T*>(dst);
                const _T *srcObj = reinterpret_cast<const _T*>(src);
                *dstObj = *srcObj;
            }
        }

        static void CopyConstruct(sharpen::Size typeIndex,char *dst,const char *src)
        {
            using _T = _TIterator<0>;
            if (0 == typeIndex)
            {
                const _T *srcObj = reinterpret_cast<const _T*>(src);
                new (dst) _T{*srcObj};
            }
        }
    };

    template<sharpen::Size _Index,template<sharpen::Size> class _TIterator>
    struct InternalDummyTypeMove
    {
        static void MoveAssign(sharpen::Size typeIndex,char *dst,char *src) noexcept
        {
            using _T = _TIterator<_Index>;
            if (_Index == typeIndex)
            {
                _T *dstObj = reinterpret_cast<_T*>(dst);
                _T *srcObj = reinterpret_cast<_T*>(src);
                *dstObj = std::move(*srcObj);
                return;
            }
            sharpen::InternalDummyTypeMove<_Index - 1,_TIterator>::MoveAssign(typeIndex,dst,src);
        }

        static void MoveConstruct(sharpen::Size typeIndex,char *dst,char *src) noexcept
        {
            using _T = _TIterator<_Index>;
            if (_Index == typeIndex)
            {
                _T *srcObj = reinterpret_cast<_T*>(src);
                new (dst) _T{std::move(*srcObj)};
                return;
            }
            sharpen::InternalDummyTypeMove<_Index - 1,_TIterator>::MoveConstruct(typeIndex,dst,src);
        }
    };

    template<template<sharpen::Size> class _TIterator>
    struct InternalDummyTypeMove<0,_TIterator>
    {
        static void MoveAssign(sharpen::Size typeIndex,char *dst,char *src) noexcept
        {
            using _T = _TIterator<0>;
            if (0 == typeIndex)
            {
                _T *dstObj = reinterpret_cast<_T*>(dst);
                _T *srcObj = reinterpret_cast<_T*>(src);
                *dstObj = std::move(*srcObj);
                return;
            }
        }

        static void MoveConstruct(sharpen::Size typeIndex,char *dst,char *src) noexcept
        {
            using _T = _TIterator<0>;
            if (0 == typeIndex)
            {
                _T *srcObj = reinterpret_cast<_T*>(src);
                new (dst) _T{std::move(*srcObj)};
                return;
            }
        }
    };

    template<typename _T,typename ..._Types>
    class DummyType
    {
    private:
        using Self = sharpen::DummyType<_T,_Types...>;
        using TL = sharpen::TypeList<_T,_Types...>;
        template<sharpen::Size _Index>
        using At = sharpen::InternalTypeListAt<TL,_Index>;
        template<typename _U>
        using Find = sharpen::InternalTypeListFind<TL,_U,_T>;
        template<typename _U>
        using Contain = sharpen::BoolType<Find<_U>::Index != TL::Size>;

        constexpr static sharpen::Size typeSize_ = sharpen::MaxValue<sharpen::Size,sizeof(_T),sizeof(_Types)...>::Value;
        constexpr static sharpen::Size typeListSize_ = TL::Size;

        char dummy_[typeSize_];
        sharpen::Size typeIndex_;

        void Release() noexcept
        {
            if(this->typeIndex_ != typeListSize_)
            {
                sharpen::InternalDummyTypeDtor<typeListSize_ - 1,At>::Release(this->typeIndex_,this->dummy_);
                this->typeIndex_ = typeListSize_;
            }
        }        
    public:
        DummyType()
            :dummy_()
            ,typeIndex_(typeListSize_)
        {}

        DummyType(const Self &other)
            :dummy_()
            ,typeIndex_(other.typeIndex_)
        {
            if(this->typeIndex_ != typeListSize_)
            {
                sharpen::InternalDummyTypeCopy<typeListSize_ - 1,At>::CopyConstruct(this->typeIndex_,this->dummy_,other.dummy_);
            }
        }

        DummyType(Self &&other) noexcept
            :dummy_()
            ,typeIndex_(other.typeIndex_)
        {
            if(this->typeIndex_ != typeListSize_)
            {
                sharpen::InternalDummyTypeMove<typeListSize_ - 1,At>::MoveConstruct(this->typeIndex_,this->dummy_,other.dummy_);
                other.Release();
            }
        }

        Self &operator=(const Self &other)
        {
            if(this != std::addressof(other) && other.typeIndex_ != typeListSize_)
            {
                Self tmp(other);
                std::swap(tmp,*this);
            }
            return *this;
        }

        Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other) && other.typeIndex_ != typeListSize_)
            {
                this->typeIndex_ = other.typeIndex_;
                sharpen::InternalDummyTypeMove<typeListSize_ - 1,At>::MoveAssign(this->typeIndex_,this->dummy_,other.dummy_);
                other.Release();
            }
            return *this;
        }

        ~DummyType() noexcept
        {
            this->Release();
        }

        template<typename _U,typename ..._Args,typename _Contain = Contain<_U>,typename _Check = sharpen::EnableIf<_Contain::Value>>
        void Construct(_Args &&...args) SHARPEN_NOEXCEPT_IF(new (nullptr) _U(std::declval<_Args>()...))
        {
            this->Release();
            new (this->dummy_) _U(std::forward<_Args>(args)...);
            this->typeIndex_ = Find<_U>::Index;
        }

        template<typename _U,typename _Contain =  Contain<_U>,typename _Check = sharpen::EnableIf<_Contain::Value>>
        _U &Get() noexcept
        {
            assert(Find<_U>::Index == this->typeIndex_);
            char *p = this->dummy_;
            return *reinterpret_cast<_U*>(p);
        }

        template<typename _U,typename _Contain =  Contain<_U>,typename _Check = sharpen::EnableIf<_Contain::Value>>
        const _U &Get() const noexcept
        {
            assert(Find<_U>::Index == this->typeIndex_);
            const char *p = this->dummy_;
            return *reinterpret_cast<const _U*>(p);
        }
    };
}

#endif