#include <sharpen/Ipv6EndPoint.hpp>

#include <cstring>
#include <utility>
#include <cassert>

#ifdef SHARPEN_IS_NIX
#include <arpa/inet.h>
#endif

#include <sharpen/ByteBuffer.hpp>

sharpen::Ipv6EndPoint::Ipv6EndPoint() noexcept
    :addr_()
{
    this->addr_.sin6_family = AF_INET6;
}

sharpen::Ipv6EndPoint::Ipv6EndPoint(const in6_addr &addr,sharpen::UintPort port)
    :addr_()
{
    this->addr_.sin6_family = AF_INET6;
    sharpen::Ipv6EndPoint::CopyIn6Addr(this->addr_.sin6_addr,addr);
    this->addr_.sin6_port = ::htons(port);
}

sharpen::Ipv6EndPoint &sharpen::Ipv6EndPoint::operator=(const sharpen::Ipv6EndPoint &other)
{
    Self tmp(other);
    std::swap(tmp,*this);
    return *this;
}

sharpen::Ipv6EndPoint &sharpen::Ipv6EndPoint::operator=(sharpen::Ipv6EndPoint &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->addr_ = std::move(other.addr_);
    }
    return *this;
}

bool sharpen::Ipv6EndPoint::operator==(const sharpen::Ipv6EndPoint &other) const noexcept
{
    return this->GetPort() == other.GetPort() && sharpen::Ipv6EndPoint::CompareIn6Addr(this->addr_.sin6_addr,other.addr_.sin6_addr) == 0;
}

sharpen::Ipv6EndPoint::NativeAddr *sharpen::Ipv6EndPoint::GetAddrPtr() noexcept
{
    return reinterpret_cast<sockaddr*>(&this->addr_);
}

const sharpen::Ipv6EndPoint::NativeAddr *sharpen::Ipv6EndPoint::GetAddrPtr() const noexcept
{
    return reinterpret_cast<const sockaddr*>(&this->addr_);
}

sharpen::UintPort sharpen::Ipv6EndPoint::GetPort() const noexcept
{
    return ::ntohs(this->addr_.sin6_port);
}

void sharpen::Ipv6EndPoint::SetPort(sharpen::UintPort port) noexcept
{
    this->addr_.sin6_port = ::htons(port);
}

void sharpen::Ipv6EndPoint::CopyIn6Addr(in6_addr &dst,const in6_addr &src) noexcept
{
#ifdef SHARPEN_IS_WIN
    std::memcpy(dst.u.Byte,src.u.Byte,sizeof(dst.u.Byte));
#else
    std::memcpy(dst.s6_addr,src.s6_addr,sizeof(dst.s6_addr));
#endif
}
void sharpen::Ipv6EndPoint::GetAddr(in6_addr &addr) const noexcept
{
    sharpen::Ipv6EndPoint::CopyIn6Addr(addr,this->addr_.sin6_addr);
}

void sharpen::Ipv6EndPoint::SetAddr(const in6_addr &addr) noexcept
{
    sharpen::Ipv6EndPoint::CopyIn6Addr(this->addr_.sin6_addr,addr);
}

void sharpen::Ipv6EndPoint::GetAddrString(char *addrStr,sharpen::Size size) const
{
    if(::inet_ntop(AF_INET6,&(this->addr_.sin6_addr),addrStr,size) == nullptr)
    {
        sharpen::ThrowLastError();
    }
}

int sharpen::Ipv6EndPoint::CompareIn6Addr(const in6_addr &addr1,const in6_addr &addr2) noexcept
{
#ifdef SHARPEN_IS_WIN
    return std::memcmp(addr1.u.Byte,addr2.u.Byte,sizeof(addr1.u.Byte));
#else
    return std::memcmp(addr1.s6_addr,addr2.s6_addr,sizeof(addr1.s6_addr));
#endif
}

void sharpen::Ipv6EndPoint::SetAddrByString(const char *addrStr)
{
    int r = ::inet_pton(AF_INET6,addrStr,&(this->addr_.sin6_addr));
    if(r == 0)
    {
        throw std::invalid_argument("invalid address string");
    }
    else if(r == -1)
    {
        sharpen::ThrowLastError();
    }
}

sharpen::Int64 sharpen::Ipv6EndPoint::CompareWith(const Self &other) const noexcept
{
    char thiz[sizeof(in6_addr) + sizeof(sharpen::Uint16)] = {};
    char otherval[sizeof(in6_addr) + sizeof(sharpen::Uint16)] = {};
    this->GetAddr(*reinterpret_cast<in6_addr*>(thiz));
    *reinterpret_cast<sharpen::Uint16*>(thiz + sizeof(in6_addr)) = this->GetPort();
    other.GetAddr(*reinterpret_cast<in6_addr*>(otherval));
    *reinterpret_cast<sharpen::Uint16*>(otherval + sizeof(in6_addr)) = other.GetPort();
    return sharpen::BufferCompare(thiz,sizeof(thiz),otherval,sizeof(otherval));
}

sharpen::Size sharpen::Ipv6EndPoint::LoadFrom(const char *data,sharpen::Size size)
{
    if(size < sizeof(in6_addr) + sizeof(sharpen::Uint16))
    {
        throw std::invalid_argument("invalid ip endpoint buffer");
    }
    sharpen::Size offset{0};
#ifdef SHARPEN_IS_WIN
    std::memcpy(this->addr_.sin6_addr.u.Byte,data,sizeof(this->addr_.sin6_addr.u.Byte));
    offset += sizeof(this->addr_.sin6_addr.u.Byte);
#else
    std::memcpy(this->addr_.sin6_addr.s6_addr,data,sizeof(this->addr_.sin6_addr.s6_addr));
    offset += sizeof(this->addr_.sin6_addr.s6_addr);
#endif
    sharpen::Uint16 port;
    std::memcpy(&port,data + offset,sizeof(port));
    this->SetPort(port);
    offset += sizeof(port);
    return offset;
}

sharpen::Size sharpen::Ipv6EndPoint::LoadFrom(const sharpen::ByteBuffer &buf,sharpen::Size offset)
{
    assert(buf.GetSize() >= offset);
    return this->LoadFrom(buf.Data() + offset,buf.GetSize() - offset);
}

sharpen::Size sharpen::Ipv6EndPoint::UnsafeStoreTo(char *data) const noexcept
{
    sharpen::Size offset{0};
    sharpen::Uint16 port{this->GetPort()};
#ifdef SHARPEN_IS_WIN
    std::memcpy(data,this->addr_.sin6_addr.u.Byte,sizeof(this->addr_.sin6_addr.u.Byte));
    offset += sizeof(this->addr_.sin6_addr.u.Byte);
#else
    std::memcpy(data,this->addr_.sin6_addr.s6_addr,sizeof(this->addr_.sin6_addr.s6_addr));
    offset += sizeof(this->addr_.sin6_addr.s6_addr);
#endif
    std::memcpy(data + offset,&port,sizeof(port));
    offset += sizeof(port);
    return offset;
}

sharpen::Size sharpen::Ipv6EndPoint::StoreTo(char *data,sharpen::Size size) const
{
    sharpen::Size needSize{this->ComputeSize()};
    if(size < needSize)
    {
        throw std::invalid_argument("buffer too small");
    }
    return this->UnsafeStoreTo(data);
}

sharpen::Size sharpen::Ipv6EndPoint::StoreTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const
{
    assert(buf.GetSize() >= offset);
    sharpen::Size needSize{this->ComputeSize()};
    sharpen::Size size{buf.GetSize() - offset};
    if(size < needSize)
    {
        buf.Extend(needSize - size);
    }
    return this->UnsafeStoreTo(buf.Data() + offset);
}