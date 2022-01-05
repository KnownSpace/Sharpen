#include <sharpen/IpEndPoint.hpp>

#include <utility>

#include <sharpen/SystemMacro.hpp>
#include <sharpen/IntOps.hpp>

#ifdef SHARPEN_IS_WIN
#include <WS2tcpip.h>
#else
#include <arpa/inet.h>
#endif

sharpen::IpEndPoint::IpEndPoint() noexcept
    :addr_()
{
    this->addr_.sin_family = AF_INET;
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

sharpen::IpEndPoint &sharpen::IpEndPoint::operator=(const sharpen::IpEndPoint &other)
{
    Self tmp(other);
    std::swap(tmp,*this);
    return *this;
}

sharpen::IpEndPoint &sharpen::IpEndPoint::operator=(sharpen::IpEndPoint &&other) noexcept
{
    if(this != std::addressof(other))
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

void sharpen::IpEndPoint::GetAddrString(char *addrStr,sharpen::Size size) const
{
    ::inet_ntop(AF_INET,&(this->addr_.sin_addr),addrStr,size);
}

void sharpen::IpEndPoint::SetAddrByString(const char *addrStr)
{
    ::inet_pton(AF_INET,addrStr,&(this->addr_.sin_addr));
}

sharpen::Int64 sharpen::IpEndPoint::CompareWith(const Self &other) const noexcept
{
    sharpen::Int64 thiz,otherval;
    thiz = this->GetAddr() << 16;
    thiz |= this->GetPort();
    otherval = other.GetAddr() << 16;
    otherval |= other.GetPort();
    return thiz - otherval;
}