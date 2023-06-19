#pragma once
#ifndef _SHARPEN_INETSTEAMFACTORY_HPP
#define _SHARPEN_INETSTEAMFACTORY_HPP

#include "INetStreamChannel.hpp"
#include "TcpStreamOption.hpp"

namespace sharpen {
    class ITcpSteamFactory {
    private:
        using Self = sharpen::ITcpSteamFactory;

    protected:
        virtual sharpen::NetStreamChannelPtr NviProduce(sharpen::TcpStreamOption option) = 0;

    public:
        ITcpSteamFactory() noexcept = default;

        ITcpSteamFactory(const Self &other) noexcept = default;

        ITcpSteamFactory(Self &&other) noexcept = default;

        Self &operator=(const Self &other) noexcept = default;

        Self &operator=(Self &&other) noexcept = default;

        virtual ~ITcpSteamFactory() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        inline sharpen::NetStreamChannelPtr Produce(sharpen::TcpStreamOption option) {
            return this->NviProduce(option);
        }

        virtual sharpen::IEventLoopGroup &GetLoopGroup() const noexcept = 0;
    };
}   // namespace sharpen

#endif