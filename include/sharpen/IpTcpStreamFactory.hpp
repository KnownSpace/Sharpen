#pragma once
#ifndef _SHARPEN_IPNETSTREAMFACTORY_HPP
#define _SHARPEN_IPNETSTREAMFACTORY_HPP

#include "IEventLoopGroup.hpp"
#include "ITcpSteamFactory.hpp"
#include "IpEndPoint.hpp"

namespace sharpen {
    class IpTcpStreamFactory : public sharpen::ITcpSteamFactory {
    private:
        using Self = sharpen::IpTcpStreamFactory;

        sharpen::IEventLoopGroup *loopGroup_;
        sharpen::IpEndPoint localEndpoint_;

        virtual sharpen::NetStreamChannelPtr NviProduce() override;

    public:
        explicit IpTcpStreamFactory(const sharpen::IpEndPoint &endpoint);

        IpTcpStreamFactory(sharpen::IEventLoopGroup &loopGroup,
                           const sharpen::IpEndPoint &endpoint) noexcept;

        IpTcpStreamFactory(const Self &other) = default;

        IpTcpStreamFactory(Self &&other) noexcept
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

        ~IpTcpStreamFactory() noexcept = default;

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