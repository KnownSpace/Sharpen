#pragma once
#ifndef _SHARPEN_IPENDPOINT_HPP
#define _SHARPEN_IPENDPOINT_HPP

#include <functional>

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_NIX
#include <netinet/in.h>
#endif

#include "IEndPoint.hpp"
#include "TypeDef.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"


namespace sharpen
{
    class IpEndPoint:public sharpen::IEndPoint
    {
    private:
        using MyAddr = sockaddr_in;
        using MyBase = sharpen::IEndPoint;
        using Self = sharpen::IpEndPoint;

        MyAddr addr_;
    public:
        IpEndPoint() noexcept;

        IpEndPoint(const Self &other) = default;

        IpEndPoint(Self &&other) noexcept = default;

        IpEndPoint(sharpen::UintIpAddr addr,sharpen::UintPort port);

        ~IpEndPoint() noexcept = default;

        Self &operator=(const Self &other);

        Self &operator=(Self &&other) noexcept;

        virtual NativeAddr *GetAddrPtr() noexcept override;

        virtual const NativeAddr *GetAddrPtr() const noexcept override;

        sharpen::UintPort GetPort() const noexcept;

        void SetPort(sharpen::UintPort port) noexcept;

        sharpen::UintIpAddr GetAddr() const noexcept;

        void SetAddr(sharpen::UintIpAddr addr) noexcept;

        void GetAddrSring(char *addrStr,sharpen::Size size) const;

        void SetAddrByString(const char *addrStr);

        virtual sharpen::Uint32 GetAddrLen() const override
        {
            return sizeof(this->addr_);
        }
    };
}

namespace std
{
    template<>
    struct hash<sharpen::IpEndPoint>
    {
        std::size_t operator()(const sharpen::IpEndPoint &endpoint) const noexcept
        {
            return endpoint.GetAddr() ^ endpoint.GetPort();
        }
    };
}
#endif