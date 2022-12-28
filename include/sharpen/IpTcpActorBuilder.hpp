#pragma once
#ifndef _SHARPEN_IPTCPACTORBUILDER_HPP
#define _SHARPEN_IPTCPACTORBUILDER_HPP

#include <stdexcept>

#include "IRemoteActor.hpp"
#include "IRemoteActorBuilder.hpp"
#include "ITcpSteamFactory.hpp"
#include "IpEndPoint.hpp"
#include "IMailReceiver.hpp"
#include "IMailParserFactory.hpp"

namespace sharpen
{
    class IpTcpActorBuilder:public sharpen::IRemoteActorBuilder
    {
    private:
        using Self = sharpen::IpTcpActorBuilder;
    
        sharpen::IpEndPoint remote_;
        sharpen::IFiberScheduler *scheduler_;
        std::shared_ptr<sharpen::ITcpSteamFactory> factory_;
        sharpen::IMailReceiver *receiver_;
        std::shared_ptr<sharpen::IMailParserFactory> parserFactory_;

        void EnsureConfiguration() const;
    public:

        IpTcpActorBuilder(sharpen::IFiberScheduler &scheduler,sharpen::IEventLoopGroup &loopGroup);
    
        IpTcpActorBuilder(const Self &other) = default;
    
        IpTcpActorBuilder(Self &&other) noexcept;
    
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
    
        virtual ~IpTcpActorBuilder() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        void SetRemote(const sharpen::IpEndPoint &remote) noexcept;

        void SetReceiver(sharpen::IMailReceiver &receiver) noexcept;

        void SetParserFactory(std::shared_ptr<sharpen::IMailParserFactory> parserFactory) noexcept;

        virtual std::unique_ptr<sharpen::IRemoteActor> Build() override;

        virtual std::shared_ptr<sharpen::IRemoteActor> BuildShared() override;
    };
}

#endif