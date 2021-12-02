#pragma once
#ifndef _SHARPEN_INTOPS_HPP
#define _SHARPEN_INTOPS_HPP

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

    template<typename _T,typename _Check = sharpen::EnableIf<std::is_integral<_T>::value>>
    struct Adder
    {
        constexpr _T operator()(_T a,_T b) const noexcept
        {
            return a + b;
        }
    };


    template<typename _T,typename _Check = sharpen::EnableIf<std::is_integral<_T>::value>>
    struct Multiplier
    {
        constexpr _T operator()(_T a,_T b) const noexcept
        {
            return a * b;
        }
    };

    template<typename _T,typename _Check = sharpen::EnableIf<std::is_integral<_T>::value>>
    struct Suber
    {
        constexpr _T operator()(_T a,_T b) const noexcept
        {
            return a - b;
        }
    };

    template<typename _T,typename _Check = sharpen::EnableIf<std::is_integral<_T>::value>>
    struct Diver
    {
        constexpr _T operator()(_T a,_T b) const noexcept
        {
            return a / b;
        }
    };

    template<typename _T,typename _Adder,typename _Check = sharpen::EnableIf<std::is_integral<_T>::value && sharpen::IsCallable<_Adder,_T,_T>::Value && std::is_unsigned<_T>::value>>
    bool InternalCheckOverflow(_T a,_T b,_Adder &&adder,int) noexcept
    {
        _T r = adder(a,b);
        return r > a && r > b;
    }

    template<typename _T,typename _Adder,typename _Check = sharpen::EnableIf<std::is_integral<_T>::value && sharpen::IsCallable<_Adder,_T,_T>::Value>>
    bool InternalCheckOverflow(_T a,_T b,_Adder &&adder,...) noexcept
    {
        _T r = adder(a,b);
        return !((r^a) < 0 && (r^b) < 0);
    }

    template<typename _T,typename _Adder,typename _Check = sharpen::EnableIf<std::is_integral<_T>::value && sharpen::IsCallable<_Adder,_T,_T>::Value>>
    bool CheckOverflow(_T a,_T b,_Adder &&adder) noexcept
    {
        return sharpen::InternalCheckOverflow(a,b,std::forward<_Adder>(adder),0);
    }
}

#endif