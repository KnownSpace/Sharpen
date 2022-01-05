#pragma once
#ifndef _SHARPEN_DNS_HPP
#define _SHARPEN_DNS_HPP

#include <memory>

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_WIN
#include <ws2tcpip.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#endif

#include "IEndPoint.hpp"
#include "ByteBuffer.hpp"
#include "SystemError.hpp"

namespace sharpen
{
    struct Dns
    {
    public:
        struct ResolveResult
        {
            sharpen::AddressFamily af_;
            std::unique_ptr<sharpen::IEndPoint> endPoint_;
            sharpen::ByteBuffer canonname_;
        };
    private:
        static int InternalResolveName(const char *name,addrinfo **addrInfos,bool useHint,int af,int sockType,int protocol) noexcept;

        static ResolveResult ConvertAddrInfoToResolveResult(addrinfo *addrInfo);

        template<typename _InsertIterator,typename _Check = decltype(std::declval<_InsertIterator>() = std::declval<ResolveResult>())>
        inline static void ResolveName(const char *name,_InsertIterator insertor,int af)
        {
            addrinfo *addrs{nullptr};
            //ignore error
            if(sharpen::Dns::InternalResolveName(name,&addrs,true,af,SOCK_DGRAM|SOCK_STREAM,IPPROTO_TCP|IPPROTO_UDP) != 0)
            {
                return;
            }
            try
            {
                for (auto begin = addrs; begin; begin = begin->ai_next)
                {
                    *insertor++ = sharpen::Dns::ConvertAddrInfoToResolveResult(begin);
                }
                ::freeaddrinfo(addrs);
            }
            catch(const std::exception&)
            {
                ::freeaddrinfo(addrs);
                throw;
            }
        }
    public:

        template<typename _InsertIterator,typename _Check = decltype(std::declval<_InsertIterator>() = std::declval<ResolveResult>())>
        inline static void ResolveName(const char *name,_InsertIterator insertor)
        {
            sharpen::Dns::ResolveName(name,insertor,AF_UNSPEC);
        }

        template<typename _InsertIterator,typename _Check = decltype(std::declval<_InsertIterator>() = std::declval<ResolveResult>())>
        static void ResolveName(const char *name,sharpen::AddressFamily af,_InsertIterator insertor)
        {
            int intAf = af == sharpen::AddressFamily::Ip ? AF_INET : AF_INET6;
            sharpen::Dns::ResolveName(name,insertor,intAf);
        }
    };
}

#endif