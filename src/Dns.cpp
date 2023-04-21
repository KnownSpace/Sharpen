#include <sharpen/Dns.hpp>

#include <cstring>
#include <new>

#include <sharpen/IpEndPoint.hpp>
#include <sharpen/Ipv6EndPoint.hpp>

sharpen::DnsResolveResult &sharpen::DnsResolveResult::operator=(Self &&other) noexcept
{
    if (this != std::addressof(other))
    {
        this->af_ = other.af_;
        this->endPoint_ = std::move(other.endPoint_);
        this->canonname_ = std::move(other.canonname_);
    }
    return *this;
}

int sharpen::Dns::InternalResolveName(const char *name,
                                      addrinfo **addrInfos,
                                      bool useHint,
                                      int af,
                                      int sockType,
                                      int protocol) noexcept
{
    if (useHint)
    {
        addrinfo hint;
        hint.ai_family = af;
        hint.ai_socktype = sockType;
        hint.ai_protocol = protocol;
        hint.ai_addrlen = 0;
        hint.ai_addr = nullptr;
        hint.ai_next = nullptr;
        hint.ai_canonname = nullptr;
        hint.ai_flags = AI_CANONNAME;
        return ::getaddrinfo(name, nullptr, &hint, addrInfos);
    }
    return ::getaddrinfo(name, nullptr, nullptr, addrInfos);
}

sharpen::DnsResolveResult sharpen::Dns::ConvertAddrInfoToResolveResult(addrinfo *addrInfo)
{
    sharpen::DnsResolveResult r;
    r.SetAddressFamily(addrInfo->ai_family == AF_INET ? sharpen::AddressFamily::Ip
                                                      : sharpen::AddressFamily::Ipv6);
    if (addrInfo->ai_canonname)
    {
        r.Canonname() =
            sharpen::ByteBuffer{addrInfo->ai_canonname, std::strlen(addrInfo->ai_canonname)};
    }
    if (r.GetAddressFamily() == sharpen::AddressFamily::Ip)
    {
        sockaddr_in *paddr = reinterpret_cast<sockaddr_in *>(addrInfo->ai_addr);
        std::uint32_t addr{0};
#ifdef SHARPEN_IS_WIN
        addr = paddr->sin_addr.S_un.S_addr;
#else
        addr = paddr->sin_addr.s_addr;
#endif
        r.EndPointPtr().reset(new (std::nothrow)
                                  sharpen::IpEndPoint{addr, ::ntohs(paddr->sin_port)});
        if (!r.EndPointPtr())
        {
            throw std::bad_alloc{};
        }
    }
    else
    {
        sockaddr_in6 *paddr = reinterpret_cast<sockaddr_in6 *>(addrInfo->ai_addr);
        r.EndPointPtr().reset(
            new (std::nothrow) sharpen::Ipv6EndPoint{paddr->sin6_addr, ::ntohs(paddr->sin6_port)});
        if (!r.EndPointPtr())
        {
            throw std::bad_alloc{};
        }
    }
    return r;
}