#pragma once
#ifndef _SHARPEN_IPENDPOINT_HPP
#define _SHARPEN_IPENDPOINT_HPP

#include <functional>
#include <stdexcept>
#include <cstdint>
#include <cstddef>

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_NIX
#include <netinet/in.h>
#endif

#include "IEndPoint.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "SystemError.hpp"
#include "TypeTraits.hpp"
#include "BufferOps.hpp"


namespace sharpen
{
    class ByteBuffer;

    struct IpEndPointHash
    {
        template<typename _U,typename _Check = sharpen::EnableIf<std::is_same<std::size_t,std::uint64_t>::value>>
        inline static std::uint64_t GetHashCode(const _U &ep,int) noexcept
        {
            return ep.GetHashCode64();
        }

        template<typename _U>
        inline static std::uint32_t GetHashCode(const _U &ep,...) noexcept
        {
            return ep.GetHashCode32();
        }
    };

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

        inline std::uint64_t GetHashCode64() const noexcept
        {
            std::uint64_t value{this->GetPort()};
            std::uint32_t *p{reinterpret_cast<std::uint32_t*>(&value) + 1};
            *p = this->GetAddr();
            return value;
        }

        inline std::uint32_t GetHashCode32() const noexcept
        {
            std::uint64_t hash{this->GetHashCode64()};
            return sharpen::BufferHash32(reinterpret_cast<const char*>(&hash),sizeof(hash));
        }

        inline std::size_t GetHashCode() const noexcept
        {
            return sharpen::IpEndPointHash::GetHashCode(*this,0);
        }

        inline virtual std::uint32_t VirtualGetHashCode32() const noexcept override
        {
            return this->GetHashCode32();
        }

        inline virtual std::uint64_t VirtualGetHashCode64() const noexcept override
        {
            return this->GetHashCode64();
        }

        inline virtual std::size_t VirtualGetHashCode() const noexcept override
        {
            return this->GetHashCode();
        }
    };
}

namespace std
{
    template<>
    struct hash<sharpen::IpEndPoint>
    {
    public:
        inline std::size_t operator()(const sharpen::IpEndPoint &endpoint) const noexcept
        {
            return endpoint.GetHashCode();
        }
    };
}
#endif