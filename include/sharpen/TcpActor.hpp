#pragma once
#ifndef _SHARPEN_TCPPOSTER_HPP
#define _SHARPEN_TCPPOSTER_HPP

#include "AwaitableFuture.hpp"
#include "IMailParserFactory.hpp"
#include "IMailReceiver.hpp"
#include "IRemoteActor.hpp"
#include "IRemotePoster.hpp"
#include "IWorkerGroup.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include <cassert>

namespace sharpen
{
    class TcpActor
        : public sharpen::IRemoteActor
        , public sharpen::Noncopyable
        , public sharpen::Nonmovable
    {
    private:
        using Self = sharpen::TcpActor;

        sharpen::IMailReceiver *receiver_;
        std::atomic_size_t postCount_;
        std::atomic_size_t ackCount_;
        std::shared_ptr<sharpen::IMailParserFactory> parserFactory_;
        std::function<void(sharpen::Mail)> pipelineCb_;
        std::unique_ptr<sharpen::IRemotePoster> poster_;
        std::unique_ptr<sharpen::IWorkerGroup> postWorker_;

        void DoPostShared(const sharpen::Mail *mail) noexcept;

        void DoPost(sharpen::Mail mail) noexcept;

        inline virtual std::uint64_t NviGetId() const noexcept override
        {
            assert(this->poster_);
            return this->poster_->GetId();
        }

        virtual void NviPost(sharpen::Mail mail) override;

        virtual void NviPostShared(const sharpen::Mail &mail) override;

        void DoReceive(sharpen::Mail mail) noexcept;

    public:
        TcpActor(sharpen::IFiberScheduler &scheduler,
                 sharpen::IMailReceiver &receiver,
                 std::shared_ptr<sharpen::IMailParserFactory> parserFactory,
                 std::unique_ptr<sharpen::IRemotePoster> poster);

        TcpActor(sharpen::IFiberScheduler &scheduler,
                 sharpen::IMailReceiver &receiver,
                 std::shared_ptr<sharpen::IMailParserFactory> parserFactory,
                 std::unique_ptr<sharpen::IRemotePoster> poster,
                 bool enablePipeline);

        virtual ~TcpActor() noexcept;

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        virtual sharpen::RemoteActorStatus GetStatus() const noexcept override;

        virtual void Cancel() noexcept override;

        virtual std::size_t GetPipelineCount() const noexcept override;

        virtual void Drain() noexcept override;

        virtual bool SupportPipeline() const noexcept override;
    };
}   // namespace sharpen

#endif