#pragma once
#ifndef _SHARPEN_BYTEORDER_HPP
#define _SHARPEN_BYTEORDER_HPP

#include "SystemMacro.hpp"

#define SHARPEN_LIL_ENDIAN 1234
#define SHARPEN_BIG_ENDIAN 4321

#ifdef SHARPEN_IS_LINUX
#include <endian.h>
#define SHARPEN_BYTEORDER  __BYTE_ORDER
#else
#if defined(__hppa__) || \
        defined(__m68k__) || defined(mc68000) || defined(_M_M68K) || \
        (defined(__MIPS__) && defined(__MISPEB__)) || \
        defined(__ppc__) || defined(__POWERPC__) || defined(_M_PPC) || \
        defined(__sparc__)
#define SHARPEN_BYTEORDER   SHARPEN_BIG_ENDIAN
#else
#define SHARPEN_BYTEORDER   SHARPEN_LIL_ENDIAN
#endif
#endif

#if (SHARPEN_BYTEORDER == SHARPEN_BIG_ENDIAN)
#define SHARPEN_IS_BIG_ENDIAN
#else
#define SHARPEN_IS_LIL_ENDIAN
#endif

namespace sharpen
{
    template<typename _T>
    void ConvertEndian(_T &val)
    {
        char *data = reinterpret_cast<char*>(&val);
        for (std::size_t i = 0; i < sizeof(_T)/2; i++)
        {
            char tmp = data[i];
            data[i] = data[sizeof(_T) - 1 - i];
            data[sizeof(_T) - 1 - i] = tmp;   
        }
    }

    inline void ConvertEndian(char *data,std::size_t size)
    {
        for (std::size_t i = 0,count = size/2; i != count; ++i)
        {
            char tmp = data[i];
            data[i] = data[size - 1 - i];
            data[size - 1 - i] = tmp;   
        }
    }

    constexpr bool IsBigEndian()
    {
        return SHARPEN_BYTEORDER == SHARPEN_BIG_ENDIAN;
    }

    constexpr bool IsLittleEndian()
    {
        return SHARPEN_BYTEORDER == SHARPEN_LIL_ENDIAN;
    }
}

#endif