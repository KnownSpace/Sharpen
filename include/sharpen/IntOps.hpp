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
    std::size_t MinSizeof(_T val)
    {
        const char *data = reinterpret_cast<const char *>(&val);
#ifdef SHARPEN_IS_BIG_ENDIAN
        for (std::size_t i = sizeof(val);i != 0; --i)
        {
            if (data[sizeof(val) - i] != 0)
            {
                return i;
            }
        }
#else
        for (std::size_t i = sizeof(val);i != 0; --i)
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
            unsigned char height_;
            unsigned char low_;
#else
            unsigned char low_;
            unsigned char height_;
#endif
        } union_;
        std::uint16_t value_;  
    };

    union Uint32Union
    {
        struct
        {
#ifdef SHARPEN_IS_BIG_ENDIAN
            std::uint16_t height_;
            std::uint16_t low_;
#else
            std::uint16_t low_;
            std::uint16_t height_;
#endif
        } union_;
        std::uint32_t value_;  
    };

    union Uint64Union
    {
        struct
        {
#ifdef SHARPEN_IS_BIG_ENDIAN
            std::uint32_t height_;
            std::uint32_t low_;
#else
            std::uint32_t low_;
            std::uint32_t height_;
#endif
        } union_;
        std::uint64_t value_;  
    };

    template<typename _T1,typename _T2>
    using IsSameSigned = sharpen::BoolType<(std::is_unsigned<_T1>::value && std::is_unsigned<_T2>::value) || (!std::is_unsigned<_T1>::value && !std::is_unsigned<_T2>::value)>;

    //i -> u
    template<typename _To,typename _From,typename _Check = sharpen::EnableIf<std::is_integral<_To>::value && std::is_integral<_From>::value && std::is_unsigned<_To>::value>>
    inline bool InternalCheckIntCast(_From from,...)
    {
        return from >= 0 && static_cast<typename std::make_unsigned<_From>::type>(from) <= (std::numeric_limits<_To>::max)();
    }

    //u -> i
    template<typename _To,typename _From,typename _Check = sharpen::EnableIf<std::is_integral<_To>::value && std::is_integral<_From>::value && std::is_unsigned<_From>::value>>
    inline bool InternalCheckIntCast(_From from,int,...)
    {
        return static_cast<typename std::make_unsigned<_To>::type>((std::numeric_limits<_To>::max)()) >= from;
    }

    //i1 -> i2
    //u1 -> u2
    template<typename _To,typename _From,typename _Check = sharpen::EnableIf<std::is_integral<_To>::value && std::is_integral<_From>::value && sharpen::IsSameSigned<_To,_From>::Value>>
    inline bool InternalCheckIntCast(_From from,int,int,...)
    {
        return from <= (std::numeric_limits<_To>::max)() && from >= (std::numeric_limits<_To>::min)();
    }

    //u -> i
    //sizeof(i) > sizeof(u)
    //u1 -> u2
    //sizeof(u1) <= sizeof(u2)
    template<typename _To,typename _From,typename _Check = sharpen::EnableIf<std::is_integral<_To>::value && std::is_integral<_From>::value && ((sizeof(_To) > sizeof(_From)) || ((sizeof(_To) == sizeof(_From)) && std::is_unsigned<_To>::value)) && std::is_unsigned<_From>::value>>
    constexpr inline bool InternalCheckIntCast(_From from,int,int,int,...)
    {
        return true;
    }

    //from as same as to
    template<typename _To,typename _From,typename _Check = sharpen::EnableIf<std::is_integral<_To>::value && std::is_integral<_From>::value && std::is_same<_To,_From>::value>>
    constexpr inline bool InternalCheckIntCast(_From from,int,int,int,int,...)
    {
        return true;
    }

    template<typename _To,typename _From>
    constexpr inline auto CheckIntCast(_From from) -> decltype(sharpen::InternalCheckIntCast<_To>(from,0,0,0))
    {
        return sharpen::InternalCheckIntCast<_To>(from,0,0,0,0);
    }

    //i -> u
    template<typename _To,typename _From,typename _Check = sharpen::EnableIf<std::is_integral<_To>::value && std::is_integral<_From>::value && std::is_unsigned<_To>::value>>
    inline _To InternalIntCast(_From from,...)
    {
        if(from >= 0 && static_cast<typename std::make_unsigned<_From>::type>(from) <= (std::numeric_limits<_To>::max)())
        {
            return static_cast<_To>(from);
        }
        throw std::bad_cast{};
    }

    //u -> i
    template<typename _To,typename _From,typename _Check = sharpen::EnableIf<std::is_integral<_To>::value && std::is_integral<_From>::value && std::is_unsigned<_From>::value>>
    inline _To InternalIntCast(_From from,int,...)
    {
        if(static_cast<typename std::make_unsigned<_To>::type>((std::numeric_limits<_To>::max)()) >= from)
        {
            return static_cast<_To>(from);
        }
        throw std::bad_cast{};
    }

    //i1 -> i2
    //u1 -> u2
    template<typename _To,typename _From,typename _Check = sharpen::EnableIf<std::is_integral<_To>::value && std::is_integral<_From>::value && sharpen::IsSameSigned<_To,_From>::Value>>
    inline _To InternalIntCast(_From from,int,int,...)
    {
        if(from <= (std::numeric_limits<_To>::max)() && from >= (std::numeric_limits<_To>::min)())
        {
            return static_cast<_To>(from);
        }
        throw std::bad_cast{};
    }

    //u -> i
    //sizeof(i) > sizeof(u)
    //u1 -> u2
    //sizeof(u1) <= sizeof(u2)
    template<typename _To,typename _From,typename _Check = sharpen::EnableIf<std::is_integral<_To>::value && std::is_integral<_From>::value && ((sizeof(_To) > sizeof(_From)) || ((sizeof(_To) == sizeof(_From)) && std::is_unsigned<_To>::value)) && std::is_unsigned<_From>::value>>
    constexpr inline _To InternalIntCast(_From from,int,int,int,...)
    {
        return static_cast<_To>(from);
    }

    //from as same as to
    template<typename _To,typename _From,typename _Check = sharpen::EnableIf<std::is_integral<_To>::value && std::is_integral<_From>::value && std::is_same<_To,_From>::value>>
    constexpr inline _To InternalIntCast(_From from,int,int,int,int,...)
    {
        return static_cast<_To>(from);
    }

    template<typename _To,typename _From>
    constexpr inline auto IntCast(_From from) -> decltype(sharpen::InternalIntCast<_To>(from,0,0,0))
    {
        return sharpen::InternalIntCast<_To>(from,0,0,0,0);
    }
}

#endif