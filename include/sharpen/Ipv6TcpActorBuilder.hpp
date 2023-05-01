#pragma once
#ifndef _SHARPEN_IPV6TCPACTORBUILDER_HPP
#define _SHARPEN_IPV6TCPACTORBUILDER_HPP

#include "IMailParserFactory.hpp"
#include "IMailReceiver.hpp"
#include "IRemoteActor.hpp"
#include "IRemoteActorBuilder.hpp"
#include "ITcpSteamFactory.hpp"
#include "Ipv6EndPoint.hpp"
#include <stdexcept>

namespace sharpen {
    class Ipv6TcpActorBuilder : public sharpen::IRemoteActorBuilder {
    private:
        using Self = sharpen::Ipv6TcpActorBuilder;

        sharpen::Ipv6EndPoint remote_;
        sharpen::IFiberScheduler *scheduler_;
        std::shared_ptr<sharpen::ITcpSteamFactory> factory_;
        sharpen::IMailReceiver *receiver_;
        std::shared_ptr<sharpen::IMailParserFactory> parserFactory_;
        bool pipeline_;

        void EnsureConfiguration() const;

        virtual std::unique_ptr<sharpen::IRemoteActor> NviBuild() const override;

        virtual std::shared_ptr<sharpen::IRemoteActor> NviBuildShared() const override;

    public:
        Ipv6TcpActorBuilder();

        explicit Ipv6TcpActorBuilder(bool pipeline);

        explicit Ipv6TcpActorBuilder(const sharpen::Ipv6EndPoint &local);

        explicit Ipv6TcpActorBuilder(const sharpen::Ipv6EndPoint &local, bool pipeline);

        Ipv6TcpActorBuilder(bool pipeline,
                            sharpen::IFiberScheduler &scheduler,
                            sharpen::IEventLoopGroup &loopGroup);

        Ipv6TcpActorBuilder(const sharpen::Ipv6EndPoint &local,
                            bool pipeline,
                            sharpen::IFiberScheduler &scheduler,
                            sharpen::IEventLoopGroup &loopGroup);

        Ipv6TcpActorBuilder(const Self &other) = default;

        Ipv6TcpActorBuilder(Self &&other) noexcept;

        inline Self &operator=(const Self &other) {
            if (this != std::addressof(other)) {
                Self tmp{other};
                std::swap(tmp, *this);
            }
            return *this;
        }

        Self &operator=(Self &&other) noexcept;

        virtual ~Ipv6TcpActorBuilder() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        void PrepareRemote(const sharpen::Ipv6EndPoint &remote) noexcept;

        void PrepareReceiver(sharpen::IMailReceiver &receiver) noexcept;

        void PrepareParserFactory(
            std::shared_ptr<sharpen::IMailParserFactory> parserFactory) noexcept;
    };
}   // namespace sharpen

#endif