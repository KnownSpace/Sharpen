#include <sharpen/Ipv6EndPoint.hpp>

#include <cstring>
#include <utility>

#ifdef SHARPEN_IS_NIX
#include <arpa/inet.h>
#endif

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
    this->addr_.sin6_port = port;
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
    return this->addr_.sin6_port;
}

void sharpen::Ipv6EndPoint::SetPort(sharpen::UintPort port) noexcept
{
    this->addr_.sin6_port = port;
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
    ::inet_ntop(AF_INET6,&(this->addr_.sin6_addr),addrStr,size);
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
    ::inet_pton(AF_INET6,addrStr,&(this->addr_.sin6_addr));
}