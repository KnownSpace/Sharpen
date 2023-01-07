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

#ifdef SHARPEN_IS_WIN
    constexpr sharpen::ErrorCode ErrorCancel = ERROR_OPERATION_ABORTED;
    constexpr sharpen::ErrorCode ErrorConnectionAborted = WSAECONNABORTED;
    constexpr sharpen::ErrorCode ErrorBlocking = WSAEWOULDBLOCK;
    constexpr sharpen::ErrorCode ErrorNotSocket = WSAENOTSOCK;
    constexpr sharpen::ErrorCode ErrorNotConnected = WSAENOTCONN;
    constexpr sharpen::ErrorCode ErrorInvalidHandle = ERROR_INVALID_HANDLE;
    constexpr sharpen::ErrorCode ErrorAccessDenied = ERROR_ACCESS_DENIED;
    constexpr sharpen::ErrorCode ErrorAlreadyInProgress = WSAEALREADY;
    constexpr sharpen::ErrorCode ErrorConnectionRefused = WSAECONNREFUSED;
    constexpr sharpen::ErrorCode ErrorIsConnected = WSAEISCONN;
    constexpr sharpen::ErrorCode ErrorIo = ERROR_IO_DEVICE;
    constexpr sharpen::ErrorCode ErrorToManyFiles = ERROR_TOO_MANY_OPEN_FILES;
    constexpr sharpen::ErrorCode ErrorConnectionReset = ERROR_NETNAME_DELETED;
    constexpr sharpen::ErrorCode ErrorNameTooLong = ERROR_FILENAME_EXCED_RANGE;
    constexpr sharpen::ErrorCode ErrorOutOfMemory = ERROR_OUTOFMEMORY;
    constexpr sharpen::ErrorCode ErrorFunctionNotImplemented = ERROR_NOT_SUPPORTED;
    constexpr sharpen::ErrorCode ErrorBrokenPipe = ERROR_BROKEN_PIPE;
    constexpr sharpen::ErrorCode ErrorBadSocketHandle = WSAEBADF;
    constexpr sharpen::ErrorCode ErrorBadFileHandle = ERROR_INVALID_HANDLE;
    constexpr sharpen::ErrorCode ErrorNoSpace = ERROR_DISK_FULL;
    constexpr sharpen::ErrorCode ErrorOperationNotSupport = WSAEOPNOTSUPP;
    constexpr sharpen::ErrorCode ErrorAddressInUse = WSAEADDRINUSE;
    constexpr sharpen::ErrorCode ErrorFileNotFound = ERROR_FILE_NOT_FOUND;
    constexpr sharpen::ErrorCode ErrorPathNotFound = ERROR_PATH_NOT_FOUND;
    constexpr sharpen::ErrorCode ErrorOutOfQuota = WSAEDQUOT;
    constexpr sharpen::ErrorCode ErrorHostUnreachable = WSAEHOSTUNREACH;
    constexpr sharpen::ErrorCode ErrorNetUnreachable = WSAENETUNREACH;
    constexpr sharpen::ErrorCode ErrorBadAddress = WSAEFAULT;
    constexpr sharpen::ErrorCode ErrorMessageTooLong = WSAEMSGSIZE;
    constexpr sharpen::ErrorCode ErrorNoDevice = ERROR_BAD_UNIT;
    constexpr sharpen::ErrorCode ErrorShutdown = WSAESHUTDOWN;
    constexpr sharpen::ErrorCode ErrorTimeout = WSAETIMEDOUT;
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
    constexpr sharpen::ErrorCode ErrorInvalidHandle = EBADF;
    constexpr sharpen::ErrorCode ErrorAccessDenied = EACCES;
    constexpr sharpen::ErrorCode ErrorAlreadyInProgress = EALREADY;
    constexpr sharpen::ErrorCode ErrorConnectionRefused = ECONNREFUSED;
    constexpr sharpen::ErrorCode ErrorIsConnected = EISCONN;
    constexpr sharpen::ErrorCode ErrorIo = EIO;
    constexpr sharpen::ErrorCode ErrorToManyFiles = EMFILE;
    constexpr sharpen::ErrorCode ErrorConnectionReset = ECONNRESET;
    constexpr sharpen::ErrorCode ErrorNameTooLong = ENAMETOOLONG;
    constexpr sharpen::ErrorCode ErrorOutOfMemory = ENOMEM;
    constexpr sharpen::ErrorCode ErrorFunctionNotImplemented = ENOSYS;
    constexpr sharpen::ErrorCode ErrorBrokenPipe = EPIPE;
    constexpr sharpen::ErrorCode ErrorBadSocketHandle = EBADF;
    constexpr sharpen::ErrorCode ErrorBadFileHandle = EBADF;
    constexpr sharpen::ErrorCode ErrorNoSpace = ENOSPC;
    constexpr sharpen::ErrorCode ErrorOperationNotSupport = EOPNOTSUPP;
    constexpr sharpen::ErrorCode ErrorAddressInUse = EADDRINUSE;
    constexpr sharpen::ErrorCode ErrorFileNotFound = ENOENT;
    constexpr sharpen::ErrorCode ErrorPathNotFound = ENOENT;
    constexpr sharpen::ErrorCode ErrorOutOfQuota = EDQUOT;
    constexpr sharpen::ErrorCode ErrorHostUnreachable = EHOSTUNREACH;
    constexpr sharpen::ErrorCode ErrorNetUnreachable = ENETUNREACH;
    constexpr sharpen::ErrorCode ErrorBadAddress = EFAULT;
    constexpr sharpen::ErrorCode ErrorMessageTooLong = EMSGSIZE;
    constexpr sharpen::ErrorCode ErrorNoDevice = ENODEV;
    constexpr sharpen::ErrorCode ErrorShutdown = ESHUTDOWN;
    constexpr sharpen::ErrorCode ErrorTimeout = ETIMEDOUT;
#endif

    inline bool IsFatalError(sharpen::ErrorCode code) noexcept
    {
        return code == sharpen::ErrorIo 
                || code == sharpen::ErrorOutOfMemory
                || code == sharpen::ErrorNoSpace
                || code == sharpen::ErrorNoDevice;
    }

    inline sharpen::ErrorCode GetLastError() noexcept
    {
#ifdef SHARPEN_IS_WIN
        sharpen::ErrorCode err{::GetLastError()};
        //covert to WSAE* error code
        switch(err)
        {
        case ERROR_CONNECTION_REFUSED:
            err = sharpen::ErrorConnectionRefused;
            break;
        case ERROR_CONNECTION_ABORTED:
            err = sharpen::ErrorConnectionAborted;
            break;
        case ERROR_NETNAME_DELETED:
            err = sharpen::ErrorConnectionReset;
            break;
        case ERROR_PORT_UNREACHABLE:
            err = sharpen::ErrorConnectionRefused;
            break;
        case ERROR_HOST_UNREACHABLE:
            err = sharpen::ErrorHostUnreachable;
            break;
        case ERROR_NETWORK_UNREACHABLE:
            err = sharpen::ErrorNetUnreachable;
            break;
        }
        return err;
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

    inline sharpen::ErrorCode GetErrorCode(const std::system_error &exception) noexcept
    {
        return static_cast<sharpen::ErrorCode>(exception.code().value());
    }
}

#endif
