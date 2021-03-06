#pragma once
#ifndef _SHARPEN_IPENDPOINT_HPP
#define _SHARPEN_IPENDPOINT_HPP

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
    class IpEndPoint:public sharpen::IEndPoint,public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using MyAddr = sockaddr_in;
        using MyBase = sharpen::IEndPoint;
        using Self = sharpen::IpEndPoint;

        MyAddr addr_;
    public:
        IpEndPoint() noexcept;

        IpEndPoint(sharpen::UintIpAddr addr,sharpen::UintPort port);

        ~IpEndPoint() noexcept = default;

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

#endif