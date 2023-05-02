#pragma once
#ifndef _SHARPEN_IPV6TCPSTREAMFACTORY_HPP
#define _SHARPEN_IPV6TCPSTREAMFACTORY_HPP

#include "ITcpSteamFactory.hpp"
#include "Ipv6EndPoint.hpp"

namespace sharpen {
    class Ipv6TcpStreamFactory : public sharpen::ITcpSteamFactory {
    private:
        using Self = sharpen::Ipv6TcpStreamFactory;

        sharpen::IEventLoopGroup *loopGroup_;
        sharpen::Ipv6EndPoint localEndpoint_;

        virtual sharpen::NetStreamChannelPtr NviProduce() override;

    public:
        explicit Ipv6TcpStreamFactory(const sharpen::Ipv6EndPoint &endpoint);

        Ipv6TcpStreamFactory(sharpen::IEventLoopGroup &loopGroup,
                             const sharpen::Ipv6EndPoint &endpoint) noexcept;

        Ipv6TcpStreamFactory(const Self &other) = default;

        Ipv6TcpStreamFactory(Self &&other) noexcept
            : loopGroup_(other.loopGroup_)
            , localEndpoint_(std::move(other.localEndpoint_)) {
            other.loopGroup_ = nullptr;
        }

        inline Self &operator=(const Self &other) {
            if (this != std::addressof(other)) {
                Self tmp{other};
                std::swap(tmp, *this);
            }
            return *this;
        }

        inline Self &operator=(Self &&other) noexcept {
            if (this != std::addressof(other)) {
                this->loopGroup_ = other.loopGroup_;
                this->localEndpoint_ = std::move(other.localEndpoint_);
                other.loopGroup_ = nullptr;
            }
            return *this;
        }

        ~Ipv6TcpStreamFactory() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        inline virtual sharpen::IEventLoopGroup &GetLoopGroup() const noexcept override {
            assert(this->loopGroup_ != nullptr);
            return *this->loopGroup_;
        }
    };
}   // namespace sharpen

#endif