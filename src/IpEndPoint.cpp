#include <sharpen/IpEndPoint.hpp>


#include <sharpen/ByteBuffer.hpp>
#include <sharpen/ByteOrder.hpp>
#include <sharpen/IntOps.hpp>
#include <sharpen/SystemMacro.hpp>

#ifdef SHARPEN_IS_WIN
#include <WS2tcpip.h>
#else
#include <arpa/inet.h>
#endif

#include <cassert>
#include <utility>

sharpen::IpEndPoint::IpEndPoint() noexcept
    : addr_()
{
    this->addr_.sin_family = AF_INET;
}

sharpen::IpEndPoint::IpEndPoint(std::uint32_t addr, std::uint16_t port) noexcept
    : addr_()
{
    addr_.sin_family = AF_INET;
#ifdef SHARPEN_IS_WIN
    addr_.sin_addr.S_un.S_addr = addr;
#else
    addr_.sin_addr.s_addr = addr;
#endif
    addr_.sin_port = ::htons(port);
}

sharpen::IpEndPoint &sharpen::IpEndPoint::operator=(const sharpen::IpEndPoint &other) noexcept
{
    if (this != std::addressof(other))
    {
        this->addr_ = other.addr_;
    }
    return *this;
}

sharpen::IpEndPoint &sharpen::IpEndPoint::operator=(sharpen::IpEndPoint &&other) noexcept
{
    if (this != std::addressof(other))
    {
        this->addr_ = std::move(other.addr_);
    }
    return *this;
}

bool sharpen::IpEndPoint::operator==(const sharpen::IpEndPoint &other) const noexcept
{
    return this->GetAddr() == other.GetAddr() && this->GetPort() == other.GetPort();
}

sharpen::IpEndPoint::NativeAddr *sharpen::IpEndPoint::GetAddrPtr() noexcept
{
    return reinterpret_cast<sharpen::IpEndPoint::NativeAddr *>(&(this->addr_));
}

const sharpen::IpEndPoint::NativeAddr *sharpen::IpEndPoint::GetAddrPtr() const noexcept
{
    return reinterpret_cast<const sharpen::IpEndPoint::NativeAddr *>(&(this->addr_));
}

std::uint16_t sharpen::IpEndPoint::GetPort() const noexcept
{
    return ::ntohs(this->addr_.sin_port);
}

void sharpen::IpEndPoint::SetPort(std::uint16_t port) noexcept
{
    this->addr_.sin_port = ::htons(port);
}

std::uint32_t sharpen::IpEndPoint::GetAddr() const noexcept
{
#ifdef SHARPEN_IS_WIN
    return this->addr_.sin_addr.S_un.S_addr;
#else
    return this->addr_.sin_addr.s_addr;
#endif
}

void sharpen::IpEndPoint::SetAddr(std::uint32_t addr) noexcept
{
#ifdef SHARPEN_IS_WIN
    this->addr_.sin_addr.S_un.S_addr = addr;
#else
    this->addr_.sin_addr.s_addr = addr;
#endif
}

void sharpen::IpEndPoint::GetAddrString(char *addrStr, std::size_t size) const
{
    if (::inet_ntop(AF_INET, &(this->addr_.sin_addr), addrStr, size) == nullptr)
    {
        sharpen::ThrowLastError();
    }
}

void sharpen::IpEndPoint::SetAddrByString(const char *addrStr)
{
    int r = ::inet_pton(AF_INET, addrStr, &(this->addr_.sin_addr));
    if (r == 0)
    {
        throw std::invalid_argument("invalid address string");
    }
    else if (r == -1)
    {
        sharpen::ThrowLastError();
    }
}

std::int64_t sharpen::IpEndPoint::CompareWith(const Self &other) const noexcept
{
    std::uint64_t thiz, otherval;
    thiz = this->GetAddr();
    thiz <<= 16;
    thiz |= this->GetPort();
    otherval = other.GetAddr();
    otherval <<= 16;
    otherval |= other.GetPort();
    if (thiz > otherval)
    {
        return 1;
    }
    if (thiz < otherval)
    {
        return -1;
    }
    return 0;
}

std::size_t sharpen::IpEndPoint::LoadFrom(const char *data, std::size_t size)
{
    if (size < sizeof(std::uint32_t) + sizeof(std::uint16_t))
    {
        throw std::invalid_argument("invalid ip endpoint buffer");
    }
    std::uint32_t ip{0};
    std::size_t offset{0};
    std::memcpy(&ip, data, sizeof(ip));
#if (SHARPEN_BYTEORDER != SHARPEN_LIL_ENDIAN)
    sharpen::ConvertEndian(ip);
#endif
    this->SetAddr(ip);
    offset += sizeof(ip);
    std::uint16_t port;
    std::memcpy(&port, data + offset, sizeof(port));
#if (SHARPEN_BYTEORDER != SHARPEN_LIL_ENDIAN)
    sharpen::ConvertEndian(port);
#endif
    this->SetPort(port);
    offset += sizeof(port);
    return offset;
}

std::size_t sharpen::IpEndPoint::LoadFrom(const sharpen::ByteBuffer &buf, std::size_t offset)
{
    assert(buf.GetSize() >= offset);
    return this->LoadFrom(buf.Data() + offset, buf.GetSize() - offset);
}

std::size_t sharpen::IpEndPoint::UnsafeStoreTo(char *data) const noexcept
{
    std::size_t offset{0};
    std::uint32_t ip{this->GetAddr()};
    std::uint16_t port{this->GetPort()};
#if (SHARPEN_BYTEORDER != SHARPEN_LIL_ENDIAN)
    sharpen::ConvertEndian(ip);
#endif
    std::memcpy(data, &ip, sizeof(ip));
    offset += sizeof(ip);
#if (SHARPEN_BYTEORDER != SHARPEN_LIL_ENDIAN)
    sharpen::ConvertEndian(port);
#endif
    std::memcpy(data + offset, &port, sizeof(port));
    offset += sizeof(port);
    return offset;
}

std::size_t sharpen::IpEndPoint::StoreTo(char *data, std::size_t size) const
{
    std::size_t needSize{this->ComputeSize()};
    if (size < needSize)
    {
        throw std::invalid_argument("buffer too small");
    }
    return this->UnsafeStoreTo(data);
}

std::size_t sharpen::IpEndPoint::StoreTo(sharpen::ByteBuffer &buf, std::size_t offset) const
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