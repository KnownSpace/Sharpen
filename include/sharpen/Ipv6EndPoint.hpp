#pragma once
#ifndef _SHARPEN_IPV6ENDPOINT_HPP
#define _SHARPEN_IPV6ENDPOINT_HPP

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_NIX
#include <netinet/in.h>
#endif

#ifdef SHARPEN_IS_WIN
#include <WS2tcpip.h>
#endif

#include "IEndPoint.hpp"
#include "TypeDef.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"

namespace sharpen
{
    class Ipv6EndPoint:public sharpen::IEndPoint,public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Mybase = sharpen::IEndPoint;
        using Self = sharpen::Ipv6EndPoint;
        using Myaddr = sockaddr_in6;
    
        Myaddr addr_;

        static void CopyIn6Addr(in6_addr &dst,const in6_addr &src);
    public:
        Ipv6EndPoint() noexcept;

        Ipv6EndPoint(const in6_addr &ip,sharpen::UintPort port);

        ~Ipv6EndPoint() noexcept = default;

        virtual NativeAddr *GetAddrPtr() noexcept override;

        virtual const NativeAddr *GetAddrPtr() const noexcept override;

        sharpen::UintPort GetPort() const noexcept;

        void SetPort(sharpen::UintPort port) noexcept;

        void GetAddr(in6_addr &addr) const noexcept;

        void SetAddr(const in6_addr &addr) noexcept;

        void GetAddrString(char *addrStr,sharpen::Size size) const;

        void SetAddrByString(const char *addrStr);

        virtual sharpen::Uint32 GetAddrLen() const override
        {
            return sizeof(this->addr_);
        }
    };
    
}

#endif