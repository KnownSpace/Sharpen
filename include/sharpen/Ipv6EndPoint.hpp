#pragma once
#ifndef _SHARPEN_IPV6ENDPOINT_HPP
#define _SHARPEN_IPV6ENDPOINT_HPP

#include <functional>
#include <stdexcept>

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
#include "SystemError.hpp"
#include "BufferOps.hpp"

namespace sharpen
{
    class Ipv6EndPoint:public sharpen::IEndPoint
    {
    private:
        using Mybase = sharpen::IEndPoint;
        using Self = sharpen::Ipv6EndPoint;
        using Myaddr = sockaddr_in6;
    
        Myaddr addr_;

        static void CopyIn6Addr(in6_addr &dst,const in6_addr &src) noexcept;

        static int CompareIn6Addr(const in6_addr &addr1,const in6_addr &addr2) noexcept;
    public:
        Ipv6EndPoint() noexcept;

        Ipv6EndPoint(const in6_addr &ip,sharpen::UintPort port);

        Ipv6EndPoint(const Self &other) = default;

        Ipv6EndPoint(Self &&other) noexcept = default;

        ~Ipv6EndPoint() noexcept = default;

        Self &operator=(const Self &other);

        Self &operator=(Self &&other) noexcept;

        bool operator==(const Self &other) const noexcept;

        inline bool operator!=(const Self &other) const noexcept
        {
            return !(*this == other);
        }

        sharpen::Int64 CompareWith(const Self &other) const noexcept;

        inline bool operator>(const Self &other) const noexcept
        {
            return this->CompareWith(other) > 0;
        }

        inline bool operator<(const Self &other) const noexcept
        {
            return this->CompareWith(other) < 0;
        }

        inline bool operator>=(const Self &other) const noexcept
        {
            return this->CompareWith(other) >= 0;
        }

        inline bool operator<=(const Self &other) const noexcept
        {
            return this->CompareWith(other) <= 0;
        }

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

namespace std
{
    template<>
    struct hash<sharpen::Ipv6EndPoint>
    {
        std::size_t operator()(const sharpen::Ipv6EndPoint &endpoint) const noexcept
        {
            char buffer[sizeof(in6_addr) + sizeof(sharpen::Uint16)] = {};
            endpoint.GetAddr(*reinterpret_cast<in6_addr*>(buffer));
            *reinterpret_cast<sharpen::Uint16*>(buffer + sizeof(in6_addr)) = endpoint.GetPort();
            return sharpen::BufferHash(buffer,sizeof(buffer));
        }
    };
}

#endif