#pragma once
#ifndef _SHARPEN_IENDPOINT_HPP
#define _SHARPEN_IENDPOINT_HPP

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_WIN
#include <WinSock2.h>
#else
#endif

namespace sharpen
{
    class IEndPoint
    {
    private:
        using Self = sharpen::IEndPoint;

    public:
        using NativeAddr = sockaddr;

        IEndPoint() = default;

        IEndPoint(const Self &other) = default;

        IEndPoint(Self &&other) noexcept = default;

        virtual ~IEndPoint() noexcept = default;

        virtual NativeAddr *GetAddrPtr() noexcept = 0;

        virtual const NativeAddr *GetAddrPtr() const noexcept = 0;

        virtual sharpen::Size GetAddrLen() const = 0;
    };
}

#endif