#pragma once
#ifndef _SHARPEN_DNS_HPP
#define _SHARPEN_DNS_HPP

#include <memory>

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_WIN
#include <ws2tcpip.h>
#else
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#endif

#include "ByteBuffer.hpp"
#include "IEndPoint.hpp"
#include "SystemError.hpp"

namespace sharpen
{

#ifdef SHARPEN_IS_WIN
    // netdb errors
    constexpr sharpen::ErrorCode ErrorNetDbHostNotFound = WSAHOST_NOT_FOUND;
    constexpr sharpen::ErrorCode ErrorNetDbTryAgain = WSATRY_AGAIN;
    constexpr sharpen::ErrorCode ErrorNetdbNoData = WSANO_DATA;
    constexpr sharpen::ErrorCode ErrorNetdbNoRecovery = WSANO_RECOVERY;
    // addrinfo errors
    constexpr sharpen::ErrorCode ErrorServiceNotFound = WSATYPE_NOT_FOUND;
    constexpr sharpen::ErrorCode ErrorSocketTypeNotSupport = WSAESOCKTNOSUPPORT;
#else
    // netdb errors
    constexpr sharpen::ErrorCode ErrorNetDbHostNotFound = HOST_NOT_FOUND;
    constexpr sharpen::ErrorCode ErrorNetDbTryAgain = TRY_AGAIN;
    constexpr sharpen::ErrorCode ErrorNetdbNoData = NO_DATA;
    constexpr sharpen::ErrorCode ErrorNetdbNoRecovery = NO_RECOVERY;
    // addrinfo errors
    constexpr sharpen::ErrorCode ErrorServiceNotFound = EAI_SERVICE;
    constexpr sharpen::ErrorCode ErrorSocketTypeNotSupport = EAI_SOCKTYPE;
#endif

    class DnsResolveResult
    {
    private:
        using Self = DnsResolveResult;

        sharpen::AddressFamily af_;
        std::unique_ptr<sharpen::IEndPoint> endPoint_;
        sharpen::ByteBuffer canonname_;

    public:
        DnsResolveResult() noexcept = default;

        DnsResolveResult(Self &&other) noexcept = default;

        Self &operator=(Self &&other) noexcept;

        ~DnsResolveResult() noexcept = default;

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline sharpen::AddressFamily GetAddressFamily() const noexcept
        {
            return this->af_;
        }

        inline void SetAddressFamily(sharpen::AddressFamily af) noexcept
        {
            this->af_ = af;
        }

        inline std::unique_ptr<sharpen::IEndPoint> &EndPointPtr() noexcept
        {
            return this->endPoint_;
        }

        inline const std::unique_ptr<sharpen::IEndPoint> &EndPointPtr() const noexcept
        {
            return this->endPoint_;
        }

        inline sharpen::IEndPoint &EndPoint() noexcept
        {
            assert(this->endPoint_);
            return *this->endPoint_;
        }

        inline const sharpen::IEndPoint &EndPoint() const noexcept
        {
            assert(this->endPoint_);
            return *this->endPoint_;
        }

        inline sharpen::ByteBuffer &Canonname() noexcept
        {
            return this->canonname_;
        }

        inline const sharpen::ByteBuffer &Canonname() const noexcept
        {
            return this->canonname_;
        }
    };

    struct Dns
    {
    public:
    private:
        static int InternalResolveName(const char *name,
                                       addrinfo **addrInfos,
                                       bool useHint,
                                       int af,
                                       int sockType,
                                       int protocol) noexcept;

        static sharpen::DnsResolveResult ConvertAddrInfoToResolveResult(addrinfo *addrInfo);

        template<typename _InsertIterator,
                 typename _Check = decltype(*std::declval<_InsertIterator &>()++ =
                                                std::declval<sharpen::DnsResolveResult &&>())>
        inline static void ResolveName(const char *name, _InsertIterator inserter, int af)
        {
            addrinfo *addrs{nullptr};
            if (sharpen::Dns::InternalResolveName(
                    name, &addrs, true, af, SOCK_DGRAM | SOCK_STREAM, IPPROTO_TCP | IPPROTO_UDP) !=
                0)
            {
                sharpen::ThrowLastError();
            }
            assert(addrs != nullptr);
            try
            {
                for (auto begin = addrs; begin; begin = begin->ai_next)
                {
                    *inserter++ = sharpen::Dns::ConvertAddrInfoToResolveResult(begin);
                }
                ::freeaddrinfo(addrs);
            }
            catch (const std::exception &)
            {
                ::freeaddrinfo(addrs);
                throw;
            }
        }

    public:
        template<typename _InsertIterator,
                 typename _Check = decltype(*std::declval<_InsertIterator &>()++ =
                                                std::declval<sharpen::DnsResolveResult &&>())>
        inline static void ResolveName(const char *name, _InsertIterator inserter)
        {
            sharpen::Dns::ResolveName(name, inserter, AF_UNSPEC);
        }

        template<typename _InsertIterator,
                 typename _Check = decltype(*std::declval<_InsertIterator &>()++ =
                                                std::declval<sharpen::DnsResolveResult &&>())>
        static void ResolveName(const char *name,
                                sharpen::AddressFamily af,
                                _InsertIterator inserter)
        {
            int intAf = af == sharpen::AddressFamily::Ip ? AF_INET : AF_INET6;
            sharpen::Dns::ResolveName(name, inserter, intAf);
        }
    };
}   // namespace sharpen

#endif