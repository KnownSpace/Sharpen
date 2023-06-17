#pragma once
#ifndef _SHARPEN_NETSTREAMACTOR_HPP
#define _SHARPEN_NETSTREAMACTOR_HPP

#include "IEndPoint.hpp"
#include "IMailParser.hpp"
#include "IRemotePoster.hpp"
#include "ITcpSteamFactory.hpp"
#include "IWorkerGroup.hpp"
#include "Noncopyable.hpp"
#include "RemotePosterOpenError.hpp"   // IWYU pragma: export

namespace sharpen {
    class TcpPoster
        : public sharpen::IRemotePoster
        , public sharpen::Noncopyable {
    private:
        using Self = sharpen::TcpPoster;

        std::unique_ptr<sharpen::SpinLock> lock_;
        sharpen::NetStreamChannelPtr channel_;
        std::unique_ptr<sharpen::IEndPoint> remoteEndpoint_;
        std::unique_ptr<sharpen::IMailParser> parser_;
        std::shared_ptr<sharpen::ITcpSteamFactory> factory_;
        std::unique_ptr<sharpen::IWorkerGroup> pipelineWorker_;

        virtual sharpen::ActorId NviGetId() const noexcept override;

        virtual sharpen::Mail NviPost(const sharpen::Mail &mail) noexcept override;

        virtual void NviPost(const sharpen::Mail &mail,
                             std::function<void(sharpen::Mail)> cb) noexcept override;

        virtual void NviClose() noexcept override;

        virtual void NviOpen(std::unique_ptr<sharpen::IMailParser> parser) override;

        sharpen::Mail DoReceive(sharpen::NetStreamChannelPtr channel) noexcept;

        void Receive(sharpen::NetStreamChannelPtr channel,
                     std::function<void(sharpen::Mail)> cb) noexcept;

    public:
        TcpPoster(std::unique_ptr<sharpen::IEndPoint> endpoint,
                  std::shared_ptr<sharpen::ITcpSteamFactory> factory);

        TcpPoster(std::unique_ptr<sharpen::IEndPoint> endpoint,
                  std::shared_ptr<sharpen::ITcpSteamFactory> factory,
                  std::unique_ptr<sharpen::IWorkerGroup> pipelineWorker);

        TcpPoster(Self &&other) noexcept = default;

        virtual ~TcpPoster() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        Self &operator=(Self &&other) noexcept;

        virtual bool Available() const noexcept override;

        virtual bool SupportPipeline() const noexcept override;
    };
}   // namespace sharpen

#endif