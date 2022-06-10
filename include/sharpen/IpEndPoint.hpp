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
#include <cstdint>
#include <cstddef>
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

        IpEndPoint(std::uint32_t addr,std::uint16_t port);

        ~IpEndPoint() noexcept = default;

        Self &operator=(const Self &other);

        Self &operator=(Self &&other) noexcept;

        bool operator==(const Self &other) const noexcept;

        inline bool operator!=(const Self &other) const noexcept
        {
            return !(*this == other);
        }

        std::int64_t CompareWith(const Self &other) const noexcept;

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

        std::uint16_t GetPort() const noexcept;

        void SetPort(std::uint16_t port) noexcept;

        std::uint32_t GetAddr() const noexcept;

        void SetAddr(std::uint32_t addr) noexcept;

        void GetAddrString(char *addrStr,std::size_t size) const;

        void SetAddrByString(const char *addrStr);

        virtual std::uint32_t GetAddrLen() const override
        {
            return sizeof(this->addr_);
        }

        constexpr static std::size_t ComputeSize() noexcept
        {
            return sizeof(std::uint32_t) + sizeof(std::uint16_t);
        }

        std::size_t LoadFrom(const char *data,std::size_t size);

        std::size_t LoadFrom(const sharpen::ByteBuffer &buf,std::size_t offset);

        inline std::size_t LoadFrom(const sharpen::ByteBuffer &buf)
        {
            return this->LoadFrom(buf,0);
        }

        std::size_t UnsafeStoreTo(char *data) const noexcept;

        std::size_t StoreTo(char *data,std::size_t size) const;

        std::size_t StoreTo(sharpen::ByteBuffer &buf,std::size_t offset) const;

        inline std::size_t StoreTo(sharpen::ByteBuffer &buf) const
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