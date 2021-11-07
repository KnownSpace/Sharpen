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
    template<sharpen::Size _Index,typename _TL>
    struct InternalDummyTypeDtor
    {
        static void Release(sharpen::Size typeIndex,char *p) noexcept
        {
            using _T = typename _TL::At<_Index>;
            if (_Index == typeIndex)
            {
                _T *obj = reinterpret_cast<_T*>(p);
                obj->~_T();
                return;
            }
            sharpen::InternalDummyTypeDtor<_Index - 1,_TL>::Release(typeIndex,p);
        }
    };
    
    template<typename _TL>
    struct InternalDummyTypeDtor<0,_TL>
    {
        static void Release(sharpen::Size typeIndex,char *p) noexcept
        {
            using _T = typename _TL::At<0>;
            if (0 == typeIndex)
            {
                _T *obj = reinterpret_cast<_T*>(p);
                obj->~_T();
            }
        }
    };

    template<sharpen::Size _Index,typename _TL>
    struct InternalDummyTypeCopy
    {
        static void CopyAssign(sharpen::Size typeIndex,char *dst,const char *src)
        {
            using _T = typename _TL::At<_Index>;
            if (_Index == typeIndex)
            {
                _T *dstObj = reinterpret_cast<_T*>(dst);
                const _T *srcObj = reinterpret_cast<const _T*>(src);
                *dstObj = *srcObj;
                return;
            }
            sharpen::InternalDummyTypeCopy<_Index - 1,_TL>::CopyAssign(typeIndex,dst,src);
        }

        static void CopyConstruct(sharpen::Size typeIndex,char *dst,const char *src)
        {
            using _T = typename _TL::At<_Index>;
            if (_Index == typeIndex)
            {
                const _T *srcObj = reinterpret_cast<const _T*>(src);
                new (dst) _T{*srcObj};
                return;
            }
            sharpen::InternalDummyTypeCopy<_Index - 1,_TL>::CopyConstruct(typeIndex,dst,src);
        }
    };
    
    template<typename _TL>
    struct InternalDummyTypeCopy<0,_TL>
    {
        static void CopyAssign(sharpen::Size typeIndex,char *dst,const char *src)
        {
            using _T = typename _TL::At<0>;
            if (0 == typeIndex)
            {
                _T *dstObj = reinterpret_cast<_T*>(dst);
                const _T *srcObj = reinterpret_cast<const _T*>(src);
                *dstObj = *srcObj;
            }
        }

        static void CopyConstruct(sharpen::Size typeIndex,char *dst,const char *src)
        {
            using _T = typename _TL::At<0>;
            if (0 == typeIndex)
            {
                const _T *srcObj = reinterpret_cast<const _T*>(src);
                new (dst) _T{*srcObj};
            }
        }
    };

    template<sharpen::Size _Index,typename _TL>
    struct InternalDummyTypeMove
    {
        static void MoveAssign(sharpen::Size typeIndex,char *dst,char *src) noexcept
        {
            using _T = typename _TL::At<_Index>;
            if (_Index == typeIndex)
            {
                _T *dstObj = reinterpret_cast<_T*>(dst);
                _T *srcObj = reinterpret_cast<_T*>(src);
                *dstObj = std::move(*srcObj);
                return;
            }
            sharpen::InternalDummyTypeMove<_Index - 1,_TL>::MoveAssign(typeIndex,dst,src);
        }

        static void MoveConstruct(sharpen::Size typeIndex,char *dst,char *src) noexcept
        {
            using _T = typename _TL::At<_Index>;
            if (_Index == typeIndex)
            {
                _T *srcObj = reinterpret_cast<_T*>(src);
                new (dst) _T{std::move(*srcObj)};
                return;
            }
            sharpen::InternalDummyTypeMove<_Index - 1,_TL>::MoveConstruct(typeIndex,dst,src);
        }
    };

    template<typename _TL>
    struct InternalDummyTypeMove<0,_TL>
    {
        static void MoveAssign(sharpen::Size typeIndex,char *dst,char *src) noexcept
        {
            using _T = typename _TL::At<0>;
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
            using _T = typename _TL::At<0>;
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
        constexpr static sharpen::Size typeSize_ = sharpen::MaxValue<sharpen::Size,sizeof(_T),sizeof(_Types)...>::Value;
        constexpr static sharpen::Size typeListSize_ = TL::Size;

        char dummy_[typeSize_];
        sharpen::Size typeIndex_;

        void Release() noexcept
        {
            if(this->typeIndex_ != typeListSize_)
            {
                sharpen::InternalDummyTypeDtor<typeListSize_ - 1,TL>::Release(this->typeIndex_,this->dummy_);
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
                sharpen::InternalDummyTypeCopy<typeListSize_ - 1,TL>::CopyConstruct(this->typeIndex_,this->dummy_,other.dummy_);
            }
        }

        DummyType(Self &&other) noexcept
            :dummy_()
            ,typeIndex_(other.typeIndex_)
        {
            if(this->typeIndex_ != typeListSize_)
            {
                sharpen::InternalDummyTypeMove<typeListSize_ - 1,TL>::MoveConstruct(this->typeIndex_,this->dummy_,other.dummy_);
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
                sharpen::InternalDummyTypeMove<typeListSize_ - 1,TL>::MoveAssign(this->typeIndex_,this->dummy_,other.dummy_);
                other.Release();
            }
            return *this;
        }

        ~DummyType() noexcept
        {
            this->Release();
        }

        template<typename _U,typename ..._Args,typename _Contain = typename TL::Contain<_U>,typename _Check = sharpen::EnableIf<_Contain::Value>>
        void Construct(_Args &&...args) SHARPEN_NOEXCEPT_IF(new (nullptr) _U(std::declval<_Args>()...))
        {
            using Find = typename TL::Find<_U>;
            this->Release();
            new (this->dummy_) _U(std::forward<_Args>(args)...);
            this->typeIndex_ = Find::Index;
        }

        template<typename _U,typename _Contain = typename TL::Contain<_U>,typename _Check = sharpen::EnableIf<_Contain::Value>>
        _U &Get() noexcept
        {
            using Find = typename TL::Find<_U>;
            assert(Find::Index == this->typeIndex_);
            char *p = this->dummy_;
            return *reinterpret_cast<_U*>(p);
        }

        template<typename _U,typename _Contain = typename TL::Contain<_U>,typename _Check = sharpen::EnableIf<_Contain::Value>>
        const _U &Get() const noexcept
        {
            using Find = typename TL::Find<_U>;
            assert(Find::Index == this->typeIndex_);
            const char *p = this->dummy_;
            return *reinterpret_cast<const _U*>(p);
        }
    };
}

#endif