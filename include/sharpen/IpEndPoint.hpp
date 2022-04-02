#pragma once
#ifndef _SHARPEN_IPENDPOINT_HPP
#define _SHARPEN_IPENDPOINT_HPP

#include <functional>
#include <stdexcept>

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_NIX
#include <netinet/in.h>
#endif

#include "IEndPoint.hpp"
#include "TypeDef.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "SystemError.hpp"


namespace sharpen
{
    class ByteBuffer;

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

        sharpen::UintIpAddr GetAddr() const noexcept;

        void SetAddr(sharpen::UintIpAddr addr) noexcept;

        void GetAddrString(char *addrStr,sharpen::Size size) const;

        void SetAddrByString(const char *addrStr);

        virtual sharpen::Uint32 GetAddrLen() const override
        {
            return sizeof(this->addr_);
        }

        constexpr static sharpen::Size ComputeSize() noexcept
        {
            return sizeof(sharpen::Uint32) + sizeof(sharpen::Uint16);
        }

        sharpen::Size LoadFrom(const char *data,sharpen::Size size);

        sharpen::Size LoadFrom(const sharpen::ByteBuffer &buf,sharpen::Size offset);

        inline sharpen::Size LoadFrom(const sharpen::ByteBuffer &buf)
        {
            return this->LoadFrom(buf,0);
        }

        sharpen::Size UnsafeStoreTo(char *data) const noexcept;

        sharpen::Size StoreTo(char *data,sharpen::Size size) const;

        sharpen::Size StoreTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const;

        inline sharpen::Size StoreTo(sharpen::ByteBuffer &buf) const
        {
            return this->StoreTo(buf,0);
        }
    };
}

namespace std
{
    template<>
    struct hash<sharpen::IpEndPoint>
    {
    private:
        template<typename _U>
        static std::size_t MapToSize(_U &&ep,int) noexcept
        {
            std::uint64_t value{ep.GetPort()};
            std::uint32_t *p{reinterpret_cast<std::uint32_t*>(&value)};
            p += 1;
            *p = ep.GetAddr();
            return value;
        }

        template<typename _U>
        static std::size_t MapToSize(_U &&ep,...) noexcept
        {
            return ep.GetAddr() ^ ep.GetPort();   
        }
    public:
        std::size_t operator()(const sharpen::IpEndPoint &endpoint) const noexcept
        {
            return this->MapToSize(endpoint,0);
        }
    };
}
#endif