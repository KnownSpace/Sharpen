#pragma once
#ifndef _SHARPEN_TCPPOSTER_HPP
#define _SHARPEN_TCPPOSTER_HPP

#include <cassert>

#include "IRemoteActor.hpp"
#include "IRemotePoster.hpp"
#include "IMailReceiver.hpp"
#include "IWorkerGroup.hpp"
#include "Nonmovable.hpp"
#include "Noncopyable.hpp"
#include "AwaitableFuture.hpp"
#include "IMailParserFactory.hpp"

namespace sharpen
{
    class TcpActor:public sharpen::IRemoteActor,public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Self = sharpen::TcpActor;
    
        sharpen::IMailReceiver *receiver_;
        std::unique_ptr<sharpen::IRemotePoster> poster_;
        std::atomic<sharpen::RemoteActorStatus> status_;
        std::shared_ptr<sharpen::IMailParserFactory> parserFactory_;
        std::unique_ptr<sharpen::IWorkerGroup> postWorker_;
        std::unique_ptr<sharpen::IWorkerGroup> receiveWorker_;

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

        void Receive(sharpen::AwaitableFuture<sharpen::Mail> *futurePtr) noexcept;
    public:
    
        TcpActor(sharpen::IFiberScheduler &scheduler,sharpen::IMailReceiver &receiver,std::shared_ptr<sharpen::IMailParserFactory> parserFactory,std::unique_ptr<sharpen::IRemotePoster> poster);

        TcpActor(sharpen::IFiberScheduler &scheduler,sharpen::IMailReceiver &receiver,std::shared_ptr<sharpen::IMailParserFactory> parserFactory,std::unique_ptr<sharpen::IRemotePoster> poster,bool enablePipeline);
    
        virtual ~TcpActor() noexcept;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline virtual sharpen::RemoteActorStatus GetStatus() const noexcept override
        {
            return this->status_;
        }

        virtual void Cancel() noexcept override;
    };
}

#endif