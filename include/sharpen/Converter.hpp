#pragma once
#ifndef _SHARPEN_CONVERTER_HPP
#define _SHARPEN_CONVERTER_HPP

#include <type_traits>

#include "TypeDef.hpp"

namespace sharpen
{
    template<typename _T,typename _RawType = typename std::remove_const<typename std::remove_reference<_T>::type>::type,typename _IsNum = typename std::enable_if<std::is_integral<_RawType>::value || std::is_floating_point<_RawType>::value>::type,typename _IsUnsig = typename std::enable_if<std::is_unsigned<_RawType>::value>::type>
    auto InternalGetAbs(_T &&v,int) -> decltype(std::forward<_T>(v))
    {
        return std::forward<_T>(v);
    }

    template<typename _T,typename _RawType = typename std::remove_const<typename std::remove_reference<_T>::type>::type,typename _IsNum = typename std::enable_if<std::is_integral<_RawType>::value || std::is_floating_point<_RawType>::value>::type>
    auto InternalGetAbs(_T &&v,...) ->decltype(v < 0 ? -v:v)
    {
        return v < 0 ? -v:v;
    }

    template<typename _T>
    auto GetAbs(_T &&v) ->decltype(sharpen::InternalGetAbs(v,0))
    {
        return sharpen::InternalGetAbs(v,0);
    }

    //unsafe
    //bufSize must be checked by user
    template<typename _T,typename _RawType = typename std::remove_const<typename std::remove_reference<_T>::type>::type,typename _IsNum = typename std::enable_if<std::is_integral<_RawType>::value>::type>
    void Itoa(_T &&val,sharpen::Byte radix,char *buf)
    {
        if (!buf)
        {
            return;
        }
        const char *index = "0123456789ABCDEF";
        sharpen::Size i{0};
        _RawType num{sharpen::GetAbs(val)};
        if (val < 0)
        {
            buf[0] = '-';
            i++;
        }
        while (num)
        {
            buf[i] = index[num % radix];
            i++;
            num /= radix;
        }
        i--;
        if (buf[0] == '-')
        {
            buf += 1;
            i--;
        }
        for (sharpen::Size t = 0,size = i + 1,count = size/2;t < count;++t)
        {
            char tmp = buf[t];
            buf[t] = buf[i - t];
            buf[i - t] = tmp;
        }
    }
}
#endif