#pragma once
#ifndef _SHARPEN_IENDPOINT_HPP
#define _SHARPEN_IENDPOINT_HPP

#include "SystemMacro.hpp"
#include <cstdint>
#include <cstddef>
#include "NetTypeDef.hpp"

#ifdef SHARPEN_IS_WIN
#include <WinSock2.h>
#else
#include <sys/socket.h>
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

        virtual std::uint32_t GetAddrLen() const = 0;

        virtual std::uint32_t VirtualGetHashCode32() const noexcept = 0;

        virtual std::uint64_t VirtualGetHashCode64() const noexcept = 0;

        virtual std::size_t VirtualGetHashCode() const noexcept = 0;
    };
}

#endif