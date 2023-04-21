#pragma once
#ifndef _SHARPEN_INTOPS_HPP
#define _SHARPEN_INTOPS_HPP

#include "ByteOrder.hpp"
#include "TypeTraits.hpp"
#include <limits>
#include <stdexcept>
#include <utility>

namespace sharpen
{
    template<typename _T, typename _Check = sharpen::EnableIf<std::is_integral<_T>::value>>
    inline std::size_t MinSizeof(_T val)
    {
        const char *data = reinterpret_cast<const char *>(&val);
#ifdef SHARPEN_IS_BIG_ENDIAN
        for (std::size_t i = sizeof(val); i != 0; --i)
        {
            if (data[sizeof(val) - i] != 0)
            {
                return i;
            }
        }
#else
        for (std::size_t i = sizeof(val); i != 0; --i)
        {
            if (data[i - 1] != 0)
            {
                return i;
            }
        }
#endif
        return 1;
    }

    template<typename _T1, typename _T2>
    constexpr inline auto Max(_T1 a, _T2 b) noexcept -> decltype(false ? a : b)
    {
        return a > b ? a : b;
    }

    template<std::size_t _Size>
    union UintUnion
    {
    private:
        using Self = sharpen::UintUnion<_Size>;

        static constexpr std::size_t halfSize_{_Size / 2};

    public:
        struct
        {
#ifdef SHARPEN_IS_BIG_ENDIAN
            sharpen::UintType<_halfSize> height_;
            sharpen::UintType<_halfSize> low_;
#else
            sharpen::UintType<Self::halfSize_> low_;
            sharpen::UintType<Self::halfSize_> height_;
#endif
        } union_;

        sharpen::UintType<_Size> value_;
    };

    template<>
    union UintUnion<8>
    {
        std::uint8_t value_;
    };

    template<std::size_t _Size>
    union IntUnion
    {
    private:
        using Self = sharpen::IntUnion<_Size>;

        static constexpr std::size_t halfSize_{_Size / 2};

    public:
        struct
        {
#ifdef SHARPEN_IS_BIG_ENDIAN
            sharpen::IntType<_halfSize> height_;
            sharpen::UintType<_halfSize> low_;
#else
            sharpen::UintType<Self::halfSize_> low_;
            sharpen::IntType<Self::halfSize_> height_;
#endif
        } union_;

        sharpen::IntType<_Size> value_;
    };

    template<>
    union IntUnion<8>
    {
        std::int8_t value_;
    };

    template<typename _T1, typename _T2>
    using IsSameSigned =
        sharpen::BoolType<(std::is_unsigned<_T1>::value && std::is_unsigned<_T2>::value) ||
                          (!std::is_unsigned<_T1>::value && !std::is_unsigned<_T2>::value)>;

    // i -> u
    template<typename _To,
             typename _From,
             typename _Check =
                 sharpen::EnableIf<std::is_integral<_To>::value && std::is_integral<_From>::value &&
                                   std::is_unsigned<_To>::value>>
    inline bool InternalCheckIntCast(_From from, ...)
    {
        return from >= 0 && static_cast<typename std::make_unsigned<_From>::type>(from) <=
                                (std::numeric_limits<_To>::max)();
    }

    // u -> i
    template<typename _To,
             typename _From,
             typename _Check =
                 sharpen::EnableIf<std::is_integral<_To>::value && std::is_integral<_From>::value &&
                                   std::is_unsigned<_From>::value>>
    inline bool InternalCheckIntCast(_From from, int, ...)
    {
        return static_cast<typename std::make_unsigned<_To>::type>(
                   (std::numeric_limits<_To>::max)()) >= from;
    }

    // i1 -> i2
    // u1 -> u2
    template<typename _To,
             typename _From,
             typename _Check =
                 sharpen::EnableIf<std::is_integral<_To>::value && std::is_integral<_From>::value &&
                                   sharpen::IsSameSigned<_To, _From>::Value>>
    inline bool InternalCheckIntCast(_From from, int, int, ...)
    {
        return from <= (std::numeric_limits<_To>::max)() &&
               from >= (std::numeric_limits<_To>::min)();
    }

    // u -> i
    // sizeof(i) > sizeof(u)
    // u1 -> u2
    // sizeof(u1) <= sizeof(u2)
    template<typename _To,
             typename _From,
             typename _Check = sharpen::EnableIf<
                 std::is_integral<_To>::value && std::is_integral<_From>::value &&
                 ((sizeof(_To) > sizeof(_From)) ||
                  ((sizeof(_To) == sizeof(_From)) && std::is_unsigned<_To>::value)) &&
                 std::is_unsigned<_From>::value>>
    constexpr inline bool InternalCheckIntCast(_From from, int, int, int, ...)
    {
        (void)from;
        return true;
    }

    // from as same as to
    template<typename _To,
             typename _From,
             typename _Check =
                 sharpen::EnableIf<std::is_integral<_To>::value && std::is_integral<_From>::value &&
                                   std::is_same<_To, _From>::value>>
    constexpr inline bool InternalCheckIntCast(_From from, int, int, int, int, ...)
    {
        (void)from;
        return true;
    }

    template<typename _To, typename _From>
    constexpr inline auto CheckIntCast(_From from)
        -> decltype(sharpen::InternalCheckIntCast<_To>(from, 0, 0, 0))
    {
        return sharpen::InternalCheckIntCast<_To>(from, 0, 0, 0, 0);
    }

    // i -> u
    template<typename _To,
             typename _From,
             typename _Check =
                 sharpen::EnableIf<std::is_integral<_To>::value && std::is_integral<_From>::value &&
                                   std::is_unsigned<_To>::value>>
    inline _To InternalIntCast(_From from, ...)
    {
        if (from >= 0 && static_cast<typename std::make_unsigned<_From>::type>(from) <=
                             (std::numeric_limits<_To>::max)())
        {
            return static_cast<_To>(from);
        }
        throw std::bad_cast{};
    }

    // u -> i
    template<typename _To,
             typename _From,
             typename _Check =
                 sharpen::EnableIf<std::is_integral<_To>::value && std::is_integral<_From>::value &&
                                   std::is_unsigned<_From>::value>>
    inline _To InternalIntCast(_From from, int, ...)
    {
        if (static_cast<typename std::make_unsigned<_To>::type>(
                (std::numeric_limits<_To>::max)()) >= from)
        {
            return static_cast<_To>(from);
        }
        throw std::bad_cast{};
    }

    // i1 -> i2
    // u1 -> u2
    template<typename _To,
             typename _From,
             typename _Check =
                 sharpen::EnableIf<std::is_integral<_To>::value && std::is_integral<_From>::value &&
                                   sharpen::IsSameSigned<_To, _From>::Value>>
    inline _To InternalIntCast(_From from, int, int, ...)
    {
        if (from <= (std::numeric_limits<_To>::max)() && from >= (std::numeric_limits<_To>::min)())
        {
            return static_cast<_To>(from);
        }
        throw std::bad_cast{};
    }

    // u -> i
    // sizeof(i) > sizeof(u)
    // u1 -> u2
    // sizeof(u1) <= sizeof(u2)
    template<typename _To,
             typename _From,
             typename _Check = sharpen::EnableIf<
                 std::is_integral<_To>::value && std::is_integral<_From>::value &&
                 ((sizeof(_To) > sizeof(_From)) ||
                  ((sizeof(_To) == sizeof(_From)) && std::is_unsigned<_To>::value)) &&
                 std::is_unsigned<_From>::value>>
    constexpr inline _To InternalIntCast(_From from, int, int, int, ...)
    {
        return static_cast<_To>(from);
    }

    // from as same as to
    template<typename _To,
             typename _From,
             typename _Check =
                 sharpen::EnableIf<std::is_integral<_To>::value && std::is_integral<_From>::value &&
                                   std::is_same<_To, _From>::value>>
    constexpr inline _To InternalIntCast(_From from, int, int, int, int, ...)
    {
        return static_cast<_To>(from);
    }

    template<typename _To, typename _From>
    constexpr inline auto IntCast(_From from)
        -> decltype(sharpen::InternalIntCast<_To>(from, 0, 0, 0))
    {
        return sharpen::InternalIntCast<_To>(from, 0, 0, 0, 0);
    }
}   // namespace sharpen

#endif