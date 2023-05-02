#pragma once
#ifndef _SHARPEN_IPV6ENDPOINT_HPP
#define _SHARPEN_IPV6ENDPOINT_HPP

#include "BufferOps.hpp"
#include "IEndPoint.hpp"
#include "Noncopyable.hpp"   // IWYU pragma: keep
#include "Nonmovable.hpp"    // IWYU pragma: keep
#include "SystemError.hpp"   // IWYU pragma: keep
#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_NIX
#include <netinet/in.h>
#endif

#ifdef SHARPEN_IS_WIN
#include <WS2tcpip.h>
#endif

#include <cstddef>
#include <cstdint>
#include <functional>
#include <stdexcept>

namespace sharpen {
    class ByteBuffer;

    class Ipv6EndPoint : public sharpen::IEndPoint {
    private:
        using Mybase = sharpen::IEndPoint;
        using Self = sharpen::Ipv6EndPoint;
        using Myaddr = sockaddr_in6;

        Myaddr addr_;

        static void CopyIn6Addr(in6_addr &dst, const in6_addr &src) noexcept;

        static int CompareIn6Addr(const in6_addr &addr1, const in6_addr &addr2) noexcept;

    public:
        Ipv6EndPoint() noexcept;

        Ipv6EndPoint(const in6_addr &ip, std::uint16_t port) noexcept;

        Ipv6EndPoint(const Self &other) noexcept = default;

        Ipv6EndPoint(Self &&other) noexcept = default;

        ~Ipv6EndPoint() noexcept = default;

        Self &operator=(const Self &other) noexcept;

        Self &operator=(Self &&other) noexcept;

        bool operator==(const Self &other) const noexcept;

        inline bool operator!=(const Self &other) const noexcept {
            return !(*this == other);
        }

        std::int64_t CompareWith(const Self &other) const noexcept;

        inline bool operator>(const Self &other) const noexcept {
            return this->CompareWith(other) > 0;
        }

        inline bool operator<(const Self &other) const noexcept {
            return this->CompareWith(other) < 0;
        }

        inline bool operator>=(const Self &other) const noexcept {
            return this->CompareWith(other) >= 0;
        }

        inline bool operator<=(const Self &other) const noexcept {
            return this->CompareWith(other) <= 0;
        }

        virtual NativeAddr *GetAddrPtr() noexcept override;

        virtual const NativeAddr *GetAddrPtr() const noexcept override;

        std::uint16_t GetPort() const noexcept;

        void SetPort(std::uint16_t port) noexcept;

        void GetAddr(in6_addr &addr) const noexcept;

        void SetAddr(const in6_addr &addr) noexcept;

        void GetAddrString(char *addrStr, std::size_t size) const;

        void SetAddrByString(const char *addrStr);

        virtual std::uint32_t GetAddrLen() const override {
            return sizeof(this->addr_);
        }

        constexpr static std::size_t ComputeSize() noexcept {
            return sizeof(in6_addr) + sizeof(std::uint16_t);
        }

        std::size_t LoadFrom(const char *data, std::size_t size);

        std::size_t LoadFrom(const sharpen::ByteBuffer &buf, std::size_t offset);

        inline std::size_t LoadFrom(const sharpen::ByteBuffer &buf) {
            return this->LoadFrom(buf, 0);
        }

        std::size_t UnsafeStoreTo(char *data) const noexcept;

        std::size_t StoreTo(char *data, std::size_t size) const;

        std::size_t StoreTo(sharpen::ByteBuffer &buf, std::size_t offset) const;

        inline std::size_t StoreTo(sharpen::ByteBuffer &buf) const {
            return this->StoreTo(buf, 0);
        }

        inline virtual std::uint64_t GetHashCode64() const noexcept override {
            char buffer[sizeof(in6_addr) + sizeof(std::uint16_t)] = {};
            this->GetAddr(*reinterpret_cast<in6_addr *>(buffer));
            *reinterpret_cast<std::uint16_t *>(buffer + sizeof(in6_addr)) = this->GetPort();
            return sharpen::BufferHash64(buffer, sizeof(buffer));
        }

        inline virtual std::uint32_t GetHashCode32() const noexcept override {
            char buffer[sizeof(in6_addr) + sizeof(std::uint16_t)] = {};
            this->GetAddr(*reinterpret_cast<in6_addr *>(buffer));
            *reinterpret_cast<std::uint16_t *>(buffer + sizeof(in6_addr)) = this->GetPort();
            return sharpen::BufferHash32(buffer, sizeof(buffer));
        }

        inline virtual std::size_t GetHashCode() const noexcept override {
            char buffer[sizeof(in6_addr) + sizeof(std::uint16_t)] = {};
            this->GetAddr(*reinterpret_cast<in6_addr *>(buffer));
            *reinterpret_cast<std::uint16_t *>(buffer + sizeof(in6_addr)) = this->GetPort();
            return sharpen::BufferHash(buffer, sizeof(buffer));
        }
    };

}   // namespace sharpen

namespace std {
    template<>
    struct hash<sharpen::Ipv6EndPoint> {
        inline std::size_t operator()(const sharpen::Ipv6EndPoint &endpoint) const noexcept {
            return endpoint.GetHashCode();
        }
    };
}   // namespace std

#endif