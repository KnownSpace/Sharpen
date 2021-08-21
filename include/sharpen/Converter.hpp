#pragma once
#ifndef _SHARPEN_CONVERTER_HPP
#define _SHARPEN_CONVERTER_HPP

#include <type_traits>

#include "TypeDef.hpp"

namespace sharpen
{
    //unsafe
    //bufSize must be checked by user
    template<typename _T,typename _RawType = typename std::remove_const<typename std::remove_reference<_T>::type>::type,typename _IsNum = typename std::enable_if<std::is_integral<_RawType>::value>::type>
    static void Itoa(_T &&val,sharpen::Byte radix,char *buf)
    {
        if (!buf)
        {
            return;
        }
        const char *index = "0123456789ABCDEF";
        sharpen::Size i{0};
        _RawType num{val};
        if (val < 0)
        {
            buf[0] = '-';
            i++;
            num = -num;
        }
        while (num)
        {
            buf[i] = index[num % radix];
            i++;
            num /= radix;
        }
        sharpen::Size t{0};
        if (buf[0] == '-')
        {
            t++;
        }
        for (sharpen::Size count = (i-t)/2; t < count; t++)
        {
            char tmp = buf[t];
            buf[t] = buf[count - t];
            buf[count - t] = tmp;
        }
    }
}

#endif