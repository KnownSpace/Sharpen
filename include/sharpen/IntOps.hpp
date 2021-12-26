#pragma once
#ifndef _SHARPEN_INTOPS_HPP
#define _SHARPEN_INTOPS_HPP

#include <utility>

#include "TypeTraits.hpp"
#include "ByteOrder.hpp"

namespace sharpen
{
    template <typename _T, typename _Check = sharpen::EnableIf<std::is_integral<_T>::value>>
    sharpen::Size MinSizeof(_T val)
    {
        const char *data = reinterpret_cast<const char *>(&val);
#ifdef SHARPEN_IS_BIG_ENDIAN
        for (sharpen::Size i = 0; i < sizeof(val); ++i)
        {
            if (data[sizeof(val) - 1 - i] == 0)
            {
                return i;
            }
        }
#else
        for (sharpen::Size i = 0; i < sizeof(val); ++i)
        {
            if (data[i] == 0)
            {
                return i;
            }
        }
#endif
        return sizeof(_T);
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
}

#endif