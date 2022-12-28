#pragma once
#ifndef _SHARPEN_IPV6TCPACTORBUILDER_HPP
#define _SHARPEN_IPV6TCPACTORBUILDER_HPP

#include <stdexcept>

#include "IRemoteActor.hpp"
#include "IRemoteActorBuilder.hpp"
#include "ITcpSteamFactory.hpp"
#include "Ipv6EndPoint.hpp"
#include "IMailReceiver.hpp"
#include "IMailParserFactory.hpp"

namespace sharpen
{
    class Ipv6TcpActorBuilder:public sharpen::IRemoteActorBuilder
    {
    private:
        using Self = sharpen::Ipv6TcpActorBuilder;
    
        sharpen::Ipv6EndPoint remote_;
        sharpen::IFiberScheduler *scheduler_;
        std::shared_ptr<sharpen::ITcpSteamFactory> factory_;
        sharpen::IMailReceiver *receiver_;
        std::shared_ptr<sharpen::IMailParserFactory> parserFactory_;

        void EnsureConfiguration() const;
    public:

        Ipv6TcpActorBuilder(sharpen::IFiberScheduler &scheduler,sharpen::IEventLoopGroup &loopGroup);
    
        Ipv6TcpActorBuilder(const Self &other) = default;
    
        Ipv6TcpActorBuilder(Self &&other) noexcept;
    
        inline Self &operator=(const Self &other)
        {
            if(this != std::addressof(other))
            {
                Self tmp{other};
                std::swap(tmp,*this);
            }
            return *this;
        }
    
        Self &operator=(Self &&other) noexcept;
    
        virtual ~Ipv6TcpActorBuilder() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        void SetRemote(const sharpen::Ipv6EndPoint &remote) noexcept;

        void SetReceiver(sharpen::IMailReceiver &receiver) noexcept;

        void SetParserFactory(std::shared_ptr<sharpen::IMailParserFactory> parserFactory) noexcept;

        virtual std::unique_ptr<sharpen::IRemoteActor> Build() override;

        virtual std::shared_ptr<sharpen::IRemoteActor> BuildShared() override;
    };
}

#endif