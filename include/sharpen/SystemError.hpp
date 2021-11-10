#pragma once
#ifndef _SHARPEN_SYSTEMERROR_HPP
#define _SHARPEN_SYSTEMERROR_HPP

#include <type_traits>
#include <system_error>

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_WIN
#include <Windows.h>
#else
#include <errno.h>
#endif

namespace sharpen
{
#ifdef SHARPEN_IS_WIN
    using ErrorCode = decltype(::GetLastError());
#else
    using ErrorCode = std::remove_reference<decltype(errno)>::type;
#endif

    inline sharpen::ErrorCode GetLastError() noexcept
    {
#ifdef SHARPEN_IS_WIN
        return ::GetLastError();
#else
        return errno;
#endif
    }

    inline void ThrowSystemError(sharpen::ErrorCode err)
    {
        throw std::system_error(err,std::system_category());
    }

    inline void ThrowLastError()
    {
        sharpen::ThrowSystemError(sharpen::GetLastError());
    }
    
    inline std::exception_ptr MakeSystemErrorPtr(sharpen::ErrorCode err)
    {
        return std::make_exception_ptr(std::system_error(err,std::system_category()));
    }

    inline std::exception_ptr MakeLastErrorPtr()
    {
       return sharpen::MakeSystemErrorPtr(sharpen::GetLastError());
    }
}

#endif
