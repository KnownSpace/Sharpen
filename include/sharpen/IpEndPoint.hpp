#pragma once
#ifndef _SHARPEN_IPENDPOINT_HPP
#define _SHARPEN_IPENDPOINT_HPP

#include "IEndPoint.hpp"
#include "TypeDef.hpp"

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
        IpEndPoint(sharpen::UintIpAddr addr,sharpen::UintPort port);

        IpEndPoint(const Self &other);

        IpEndPoint(Self &&other) noexcept;

        ~IpEndPoint() noexcept = default;

        virtual NativeAddr *GetAddrPtr() noexcept override;

        virtual const NativeAddr *GetAddrPtr() const noexcept override;

        sharpen::UintPort GetPort() const noexcept;

        void SetPort(sharpen::UintPort port) noexcept;

        sharpen::UintIpAddr GetAddr() const noexcept;

        void SetAddr(sharpen::UintIpAddr addr) noexcept;

        void GetAddr(char *addrStr,sharpen::Size size) const;

        void SetAddr(const char *addrStr);
    };
}

#endif