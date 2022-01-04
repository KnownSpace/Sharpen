#pragma once
#ifndef _SHARPEN_INTOPS_HPP
#define _SHARPEN_INTOPS_HPP

#include <utility>
#include <limits>
#include <stdexcept>

#include "TypeTraits.hpp"
#include "ByteOrder.hpp"

namespace sharpen
{
    template <typename _T, typename _Check = sharpen::EnableIf<std::is_integral<_T>::value>>
    sharpen::Size MinSizeof(_T val)
    {
        const char *data = reinterpret_cast<const char *>(&val);
#ifdef SHARPEN_IS_BIG_ENDIAN
        for (sharpen::Size i = sizeof(val);i != 0; --i)
        {
            if (data[sizeof(val) - i] != 0)
            {
                return i;
            }
        }
#else
        for (sharpen::Size i = sizeof(val);i != 0; --i)
        {
            if (data[i-1] != 0)
            {
                return i;
            }
        }
#endif
        return 1;
    }

    template<typename _T1,typename _T2>
    constexpr inline auto Max(_T1 a,_T2 b) noexcept -> decltype(false?a:b)
    {
        return a>b?a:b;
    }

    union Uint16Union
    {
        struct
        {
#ifdef SHARPEN_IS_BIG_ENDIAN
            sharpen::Byte height_;
            sharpen::Byte low_;
#else
            sharpen::Byte low_;
            sharpen::Byte height_;
#endif
        } union_;
        sharpen::Uint16 value_;  
    };

    union Uint32Union
    {
        struct
        {
#ifdef SHARPEN_IS_BIG_ENDIAN
            sharpen::Uint16 height_;
            sharpen::Uint16 low_;
#else
            sharpen::Uint16 low_;
            sharpen::Uint16 height_;
#endif
        } union_;
        sharpen::Uint32 value_;  
    };

    union Uint64Union
    {
        struct
        {
#ifdef SHARPEN_IS_BIG_ENDIAN
            sharpen::Uint32 height_;
            sharpen::Uint32 low_;
#else
            sharpen::Uint32 low_;
            sharpen::Uint32 height_;
#endif
        } union_;
        sharpen::Uint64 value_;  
    };

    template<typename _To,typename _From,typename _Check = sharpen::EnableIf<std::is_integral<_To>::value && std::is_integral<_From>::value>>
    constexpr inline bool InternalCheckIntCast(_From from,...)
    {
        return from <= (std::numeric_limits<_To>::max)() && from >= (std::numeric_limits<_To>::min)();
    }

    template<typename _To,typename _From,typename _Check = sharpen::EnableIf<std::is_integral<_To>::value && std::is_integral<_From>::value && (sizeof(_To) > sizeof(_From)) && std::is_unsigned<_From>::value>>
    constexpr inline bool InternalCheckIntCast(_From from,int,...)
    {
        return true;
    }

    template<typename _To,typename _From,typename _Check = sharpen::EnableIf<std::is_integral<_To>::value && std::is_integral<_From>::value && std::is_same<_To,_From>::value>>
    constexpr inline bool InternalCheckIntCast(_From from,int,int,...)
    {
        return true;
    }

    template<typename _To,typename _From>
    constexpr inline auto CheckIntCast(_From from) -> decltype(sharpen::InternalCheckIntCast<_To>(from,0,0,0))
    {
        return sharpen::InternalCheckIntCast<_To>(from,0,0,0);
    }

    template<typename _To,typename _From,typename _Check = sharpen::EnableIf<std::is_integral<_To>::value && std::is_integral<_From>::value>>
    constexpr inline _To InternalIntCast(_From from,...)
    {
        if(!sharpen::CheckIntCast<_To>(from))
        {
            throw std::out_of_range("value cann't cast to target type");
        }
        return static_cast<_To>(from);
    }

    template<typename _To,typename _From,typename _Check = sharpen::EnableIf<std::is_integral<_To>::value && std::is_integral<_From>::value && (sizeof(_To) > sizeof(_From)) && std::is_unsigned<_From>::value>>
    constexpr inline _To InternalIntCast(_From from,int,...)
    {
        return static_cast<_To>(from);
    }

    template<typename _To,typename _From,typename _Check = sharpen::EnableIf<std::is_integral<_To>::value && std::is_integral<_From>::value && std::is_same<_To,_From>::value>>
    constexpr inline _To InternalIntCast(_From from,int,int,...)
    {
        return static_cast<_To>(from);
    }

    template<typename _To,typename _From>
    constexpr inline auto IntCast(_From from) -> decltype(sharpen::InternalIntCast<_To>(from,0,0,0))
    {
        return sharpen::InternalIntCast<_To>(from,0,0,0);
    }
}

#endif