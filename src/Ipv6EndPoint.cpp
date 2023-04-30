#include <sharpen/Ipv6EndPoint.hpp>

#include <sharpen/ByteBuffer.hpp>
#include <sharpen/ByteOrder.hpp>

#ifdef SHARPEN_IS_NIX
#include <arpa/inet.h>
#endif

#include <cassert>
#include <cstring>
#include <utility>

sharpen::Ipv6EndPoint::Ipv6EndPoint() noexcept
    : addr_()
{
    this->addr_.sin6_family = AF_INET6;
}

sharpen::Ipv6EndPoint::Ipv6EndPoint(const in6_addr &addr, std::uint16_t port) noexcept
    : addr_()
{
    this->addr_.sin6_family = AF_INET6;
    sharpen::Ipv6EndPoint::CopyIn6Addr(this->addr_.sin6_addr, addr);
    this->addr_.sin6_port = ::htons(port);
}

sharpen::Ipv6EndPoint &sharpen::Ipv6EndPoint::operator=(const sharpen::Ipv6EndPoint &other) noexcept
{
    if (this != std::addressof(other))
    {
        this->addr_ = other.addr_;
    }
    return *this;
}

sharpen::Ipv6EndPoint &sharpen::Ipv6EndPoint::operator=(sharpen::Ipv6EndPoint &&other) noexcept
{
    if (this != std::addressof(other))
    {
        this->addr_ = std::move(other.addr_);
    }
    return *this;
}

bool sharpen::Ipv6EndPoint::operator==(const sharpen::Ipv6EndPoint &other) const noexcept
{
    return this->GetPort() == other.GetPort() &&
           sharpen::Ipv6EndPoint::CompareIn6Addr(this->addr_.sin6_addr, other.addr_.sin6_addr) == 0;
}

sharpen::Ipv6EndPoint::NativeAddr *sharpen::Ipv6EndPoint::GetAddrPtr() noexcept
{
    return reinterpret_cast<sockaddr *>(&this->addr_);
}

const sharpen::Ipv6EndPoint::NativeAddr *sharpen::Ipv6EndPoint::GetAddrPtr() const noexcept
{
    return reinterpret_cast<const sockaddr *>(&this->addr_);
}

std::uint16_t sharpen::Ipv6EndPoint::GetPort() const noexcept
{
    return ::ntohs(this->addr_.sin6_port);
}

void sharpen::Ipv6EndPoint::SetPort(std::uint16_t port) noexcept
{
    this->addr_.sin6_port = ::htons(port);
}

void sharpen::Ipv6EndPoint::CopyIn6Addr(in6_addr &dst, const in6_addr &src) noexcept
{
#ifdef SHARPEN_IS_WIN
    std::memcpy(dst.u.Byte, src.u.Byte, sizeof(dst.u.Byte));
#else
    std::memcpy(dst.s6_addr, src.s6_addr, sizeof(dst.s6_addr));
#endif
}
void sharpen::Ipv6EndPoint::GetAddr(in6_addr &addr) const noexcept
{
    sharpen::Ipv6EndPoint::CopyIn6Addr(addr, this->addr_.sin6_addr);
}

void sharpen::Ipv6EndPoint::SetAddr(const in6_addr &addr) noexcept
{
    sharpen::Ipv6EndPoint::CopyIn6Addr(this->addr_.sin6_addr, addr);
}

void sharpen::Ipv6EndPoint::GetAddrString(char *addrStr, std::size_t size) const
{
    if (::inet_ntop(AF_INET6, &(this->addr_.sin6_addr), addrStr, size) == nullptr)
    {
        sharpen::ThrowLastError();
    }
}

int sharpen::Ipv6EndPoint::CompareIn6Addr(const in6_addr &addr1, const in6_addr &addr2) noexcept
{
#ifdef SHARPEN_IS_WIN
    return std::memcmp(addr1.u.Byte, addr2.u.Byte, sizeof(addr1.u.Byte));
#else
    return std::memcmp(addr1.s6_addr, addr2.s6_addr, sizeof(addr1.s6_addr));
#endif
}

void sharpen::Ipv6EndPoint::SetAddrByString(const char *addrStr)
{
    int r = ::inet_pton(AF_INET6, addrStr, &(this->addr_.sin6_addr));
    if (r == 0)
    {
        throw std::invalid_argument("invalid address string");
    }
    else if (r == -1)
    {
        sharpen::ThrowLastError();
    }
}

std::int64_t sharpen::Ipv6EndPoint::CompareWith(const Self &other) const noexcept
{
    char thiz[sizeof(in6_addr) + sizeof(std::uint16_t)] = {};
    char otherval[sizeof(in6_addr) + sizeof(std::uint16_t)] = {};
    this->GetAddr(*reinterpret_cast<in6_addr *>(thiz));
    *reinterpret_cast<std::uint16_t *>(thiz + sizeof(in6_addr)) = this->GetPort();
    other.GetAddr(*reinterpret_cast<in6_addr *>(otherval));
    *reinterpret_cast<std::uint16_t *>(otherval + sizeof(in6_addr)) = other.GetPort();
    return sharpen::BufferCompare(thiz, sizeof(thiz), otherval, sizeof(otherval));
}

std::size_t sharpen::Ipv6EndPoint::LoadFrom(const char *data, std::size_t size)
{
    if (size < sizeof(in6_addr) + sizeof(std::uint16_t))
    {
        throw std::invalid_argument("invalid ip endpoint buffer");
    }
    std::size_t offset{0};
#ifdef SHARPEN_IS_WIN
    std::memcpy(this->addr_.sin6_addr.u.Byte, data, sizeof(this->addr_.sin6_addr.u.Byte));
    offset += sizeof(this->addr_.sin6_addr.u.Byte);
#else
    std::memcpy(this->addr_.sin6_addr.s6_addr, data, sizeof(this->addr_.sin6_addr.s6_addr));
    offset += sizeof(this->addr_.sin6_addr.s6_addr);
#endif
    std::uint16_t port;
    std::memcpy(&port, data + offset, sizeof(port));
#if (SHARPEN_BYTEORDER != SHARPEN_LIL_ENDIAN)
    sharpen::ConvertEndian(port);
#endif
    this->SetPort(port);
    offset += sizeof(port);
    return offset;
}

std::size_t sharpen::Ipv6EndPoint::LoadFrom(const sharpen::ByteBuffer &buf, std::size_t offset)
{
    assert(buf.GetSize() >= offset);
    return this->LoadFrom(buf.Data() + offset, buf.GetSize() - offset);
}

std::size_t sharpen::Ipv6EndPoint::UnsafeStoreTo(char *data) const noexcept
{
    std::size_t offset{0};
    std::uint16_t port{this->GetPort()};
#ifdef SHARPEN_IS_WIN
    std::memcpy(data, this->addr_.sin6_addr.u.Byte, sizeof(this->addr_.sin6_addr.u.Byte));
    offset += sizeof(this->addr_.sin6_addr.u.Byte);
#else
    std::memcpy(data, this->addr_.sin6_addr.s6_addr, sizeof(this->addr_.sin6_addr.s6_addr));
    offset += sizeof(this->addr_.sin6_addr.s6_addr);
#endif
#if (SHARPEN_BYTEORDER != SHARPEN_LIL_ENDIAN)
    sharpen::ConvertEndian(port);
#endif
    std::memcpy(data + offset, &port, sizeof(port));
    offset += sizeof(port);
    return offset;
}

std::size_t sharpen::Ipv6EndPoint::StoreTo(char *data, std::size_t size) const
{
    std::size_t needSize{this->ComputeSize()};
    if (size < needSize)
    {
        throw std::invalid_argument("buffer too small");
    }
    return this->UnsafeStoreTo(data);
}

std::size_t sharpen::Ipv6EndPoint::StoreTo(sharpen::ByteBuffer &buf, std::size_t offset) const
{
    assert(buf.GetSize() >= offset);
    std::size_t needSize{this->ComputeSize()};
    std::size_t size{buf.GetSize() - offset};
    if (size < needSize)
    {
        buf.Extend(needSize - size);
    }
    return this->UnsafeStoreTo(buf.Data() + offset);
}