#pragma once
#ifndef _SHARPEN_IENDPOINT_HPP
#define _SHARPEN_IENDPOINT_HPP

#include "NetTypeDef.hpp" // IWYU pragma: keep
#include "SystemMacro.hpp"
#include <cstddef>
#include <cstdint>

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

        virtual std::uint32_t GetHashCode32() const noexcept = 0;

        virtual std::uint64_t GetHashCode64() const noexcept = 0;

        virtual std::size_t GetHashCode() const noexcept = 0;

        inline std::uint64_t GetActorId() const noexcept
        {
            return this->GetHashCode64();
        }
    };
}   // namespace sharpen

#endif