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
        bool pipeline_;

        void EnsureConfiguration() const;

        virtual std::unique_ptr<sharpen::IRemoteActor> NviBuild() const override;

        virtual std::shared_ptr<sharpen::IRemoteActor> NviBuildShared() const override;
    public:

        IpTcpActorBuilder();

        explicit IpTcpActorBuilder(bool pipeline);

        explicit IpTcpActorBuilder(const sharpen::IpEndPoint &local);

        IpTcpActorBuilder(const sharpen::IpEndPoint &local,bool pipeline);

        IpTcpActorBuilder(bool pipeline,sharpen::IFiberScheduler &scheduler,sharpen::IEventLoopGroup &loopGroup);

        IpTcpActorBuilder(const sharpen::IpEndPoint &local,bool pipeline,sharpen::IFiberScheduler &scheduler,sharpen::IEventLoopGroup &loopGroup);
    
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

        void PrepareRemote(const sharpen::IpEndPoint &remote) noexcept;

        void PrepareReceiver(sharpen::IMailReceiver &receiver) noexcept;

        void PrepareParserFactory(std::shared_ptr<sharpen::IMailParserFactory> parserFactory) noexcept;
    };
}

#endif