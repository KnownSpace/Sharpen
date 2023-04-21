#pragma once
#ifndef _SHARPEN_TCPHOST_HPP
#define _SHARPEN_TCPHOST_HPP

#include "AsyncBarrier.hpp"
#include "IHost.hpp"
#include "ITcpSteamFactory.hpp"

namespace sharpen
{
    class TcpHost
        : public sharpen::IHost
        , public sharpen::Noncopyable
        , public sharpen::Nonmovable
    {
    private:
        using Self = TcpHost;

        sharpen::IFiberScheduler *scheduler_;
        sharpen::IEventLoopGroup *loopGroup_;
        std::atomic_bool token_;
        std::unique_ptr<sharpen::IHostPipeline> pipeline_;
        sharpen::NetStreamChannelPtr acceptor_;

        virtual void NviSetPipeline(
            std::unique_ptr<sharpen::IHostPipeline> pipeline) noexcept override;

        void ConsumeChannel(sharpen::NetStreamChannelPtr channel,
                            std::atomic_size_t *counter) noexcept;

    public:
        explicit TcpHost(sharpen::ITcpSteamFactory &factory);

        TcpHost(sharpen::IFiberScheduler &scheduler, sharpen::ITcpSteamFactory &factory);

        virtual ~TcpHost() noexcept;

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        virtual void Run() override;

        virtual void Stop() noexcept override;
    };
}   // namespace sharpen

#endif