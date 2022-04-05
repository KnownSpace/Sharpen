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

#ifdef SHARPEN_IS_WIN
    constexpr sharpen::ErrorCode ErrorCancel = ERROR_OPERATION_ABORTED;
    constexpr sharpen::ErrorCode ErrorConnectionAborted = ERROR_CONNECTION_ABORTED;
    constexpr sharpen::ErrorCode ErrorBlocking = WSAEWOULDBLOCK;
    constexpr sharpen::ErrorCode ErrorNotSocket = WSAENOTSOCK;
    constexpr sharpen::ErrorCode ErrorNotConnected = WSAENOTCONN;
    constexpr sharpen::ErrorCode ErrorInvalidFileHandle = ERROR_INVALID_HANDLE;
    constexpr sharpen::ErrorCode ErrorAccessDenied = ERROR_ACCESS_DENIED;
    constexpr sharpen::ErrorCode ErrorAlready = WSAEALREADY;
    constexpr sharpen::ErrorCode ErrorConnectRefused = WSAECONNREFUSED;
    constexpr sharpen::ErrorCode ErrorIsConnected = WSAEISCONN;
    constexpr sharpen::ErrorCode ErrorIo = ERROR_IO_DEVICE;
    constexpr sharpen::ErrorCode ErrorToManyFiles = ERROR_TOO_MANY_OPEN_FILES;
    constexpr sharpen::ErrorCode ErrorConnectReset = WSAECONNRESET;
    constexpr sharpen::ErrorCode ErrorNameTooLong = ERROR_FILENAME_EXCED_RANGE;
    constexpr sharpen::ErrorCode ErrorNotEnoughMemory = ERROR_NOT_ENOUGH_MEMORY;
    constexpr sharpen::ErrorCode ErrorFunctionNotImplemented = ERROR_NOT_SUPPORTED;
    constexpr sharpen::ErrorCode ErrorBrokenPipe = ERROR_BROKEN_PIPE;
#else
    constexpr sharpen::ErrorCode ErrorCancel = ECANCELED;
    constexpr sharpen::ErrorCode ErrorConnectionAborted = ECONNABORTED;
#ifdef EAGAIN
    constexpr sharpen::ErrorCode ErrorBlocking = EAGAIN;
#else
    constexpr sharpen::ErrorCode ErrorBlocking = EWOULDBLOCK;
#endif
    constexpr sharpen::ErrorCode ErrorNotSocket = ENOTSOCK;
    constexpr sharpen::ErrorCode ErrorNotConnected = ENOTCONN;
    constexpr sharpen::ErrorCode ErrorInvalidFileHandle = EBADF;
    constexpr sharpen::ErrorCode ErrorAccessDenied = EACCES;
    constexpr sharpen::ErrorCode ErrorAlready = EALREADY;
    constexpr sharpen::ErrorCode ErrorConnectRefused = ECONNREFUSED;
    constexpr sharpen::ErrorCode ErrorIsConnected = EISCONN;
    constexpr sharpen::ErrorCode ErrorIo = EIO;
    constexpr sharpen::ErrorCode ErrorToManyFiles = EMFILE;
    constexpr sharpen::ErrorCode ErrorConnectReset = ECONNRESET;
    constexpr sharpen::ErrorCode ErrorNameTooLong = ENAMETOOLONG;
    constexpr sharpen::ErrorCode ErrorNotEnoughMemory = ENOMEM;
    constexpr sharpen::ErrorCode ErrorFunctionNotImplemented = ENOSYS;
    constexpr sharpen::ErrorCode ErrorBrokenPipe = EPIPE;
#endif
}

#endif
