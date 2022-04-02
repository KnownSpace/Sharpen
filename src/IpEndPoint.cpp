#include <sharpen/IpEndPoint.hpp>

#include <utility>
#include <cassert>

#include <sharpen/SystemMacro.hpp>
#include <sharpen/IntOps.hpp>
#include <sharpen/ByteBuffer.hpp>

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
    if(::inet_ntop(AF_INET,&(this->addr_.sin_addr),addrStr,size) == nullptr)
    {
        sharpen::ThrowLastError();
    }
}

void sharpen::IpEndPoint::SetAddrByString(const char *addrStr)
{
    int r = ::inet_pton(AF_INET,addrStr,&(this->addr_.sin_addr));
    if(r == 0)
    {
        throw std::invalid_argument("invalid address string");
    }
    else if(r == -1)
    {
        sharpen::ThrowLastError();
    }
}

sharpen::Int64 sharpen::IpEndPoint::CompareWith(const Self &other) const noexcept
{
    sharpen::Uint64 thiz,otherval;
    thiz = this->GetAddr() << 16;
    thiz |= this->GetPort();
    otherval = other.GetAddr() << 16;
    otherval |= other.GetPort();
    if(thiz > otherval)
    {
        return 1;
    }
    if(thiz < otherval)
    {
        return -1;
    }
    return 0;
}

sharpen::Size sharpen::IpEndPoint::LoadFrom(const char *data,sharpen::Size size)
{
    if(size < sizeof(sharpen::Uint32) + sizeof(sharpen::Uint16))
    {
        throw std::invalid_argument("invalid ip endpoint buffer");
    }
    sharpen::Uint32 ip{0};
    sharpen::Size offset{0};
    std::memcpy(&ip,data,sizeof(ip));
    this->SetAddr(ip);
    offset += sizeof(ip);
    sharpen::Uint16 port;
    std::memcpy(&port,data + offset,sizeof(port));
    this->SetPort(port);
    offset += sizeof(port);
    return offset;
}

sharpen::Size sharpen::IpEndPoint::LoadFrom(const sharpen::ByteBuffer &buf,sharpen::Size offset)
{
    assert(buf.GetSize() >= offset);
    return this->LoadFrom(buf.Data() + offset,buf.GetSize() - offset);
}

sharpen::Size sharpen::IpEndPoint::UnsafeStoreTo(char *data) const noexcept
{
    sharpen::Size offset{0};
    sharpen::Uint32 ip{this->GetAddr()};
    sharpen::Uint16 port{this->GetPort()};
    std::memcpy(data,&ip,sizeof(ip));
    offset += sizeof(ip);
    std::memcpy(data + offset,&port,sizeof(port));
    offset += sizeof(port);
    return offset;
}

sharpen::Size sharpen::IpEndPoint::StoreTo(char *data,sharpen::Size size) const
{
    sharpen::Size needSize{this->ComputeSize()};
    if(size < needSize)
    {
        throw std::invalid_argument("buffer too small");
    }
    return this->UnsafeStoreTo(data);
}

sharpen::Size sharpen::IpEndPoint::StoreTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const
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