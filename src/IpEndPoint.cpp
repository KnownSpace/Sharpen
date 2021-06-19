#include <sharpen/IpEndPoint.hpp>

#include <sharpen/SystemMacro.hpp>

#ifdef SHARPEN_IS_WIN
#include <WS2tcpip.h>
#else
#include <arpa/inet.h>
#endif

sharpen::IpEndPoint::IpEndPoint() noexcept
{
    addr_.sin_family = AF_INET;
}

sharpen::IpEndPoint::IpEndPoint(sharpen::UintIpAddr addr,sharpen::UintPort port)
    :addr_()
{
    addr_.sin_family = AF_INET;
#ifdef SHARPEN_IS_WIN
    addr_.sin_addr.S_un.S_addr = addr;
#else
    addr_.sin_addr.s_addr = addr;
#endif
    addr_.sin_port = ::htons(port);
}

sharpen::IpEndPoint::IpEndPoint(const Self &other)
    :addr_(other.addr_)
{}


sharpen::IpEndPoint::NativeAddr *sharpen::IpEndPoint::GetAddrPtr() noexcept
{
    return reinterpret_cast<sharpen::IpEndPoint::NativeAddr*>(&(this->addr_));
}

const sharpen::IpEndPoint::NativeAddr *sharpen::IpEndPoint::GetAddrPtr() const noexcept
{
    return reinterpret_cast<const sharpen::IpEndPoint::NativeAddr*>(&(this->addr_));
}

sharpen::UintPort sharpen::IpEndPoint::GetPort() const noexcept
{
    return ::ntohs(this->addr_.sin_port);
}

void sharpen::IpEndPoint::SetPort(sharpen::UintPort port) noexcept
{
    this->addr_.sin_port = ::htons(port);
}

sharpen::UintIpAddr sharpen::IpEndPoint::GetAddr() const noexcept
{
#ifdef SHARPEN_IS_WIN
    return this->addr_.sin_addr.S_un.S_addr;
#else
    return this->addr_.sin_addr.s_addr;
#endif
}

void sharpen::IpEndPoint::SetAddr(sharpen::UintIpAddr addr) noexcept
{
#ifdef SHARPEN_IS_WIN
    this->addr_.sin_addr.S_un.S_addr = addr;
#else
    this->addr_.sin_addr.s_addr = addr;
#endif
}

void sharpen::IpEndPoint::GetAddr(char *addrStr,sharpen::Size size) const
{
    ::inet_ntop(AF_INET,&(this->addr_.sin_addr),addrStr,size);
}

void sharpen::IpEndPoint::SetAddr(const char *addrStr)
{
    ::inet_pton(AF_INET,addrStr,&(this->addr_.sin_addr));
}

sharpen::IpEndPoint &sharpen::IpEndPoint::operator=(const sharpen::IpEndPoint &other)
{
    this->addr_ = other.addr_;
    return *this;
}