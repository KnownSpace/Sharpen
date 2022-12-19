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

namespace sharpen
{
    class TcpActor:public sharpen::IRemoteActor,public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Self = sharpen::TcpActor;
    
        sharpen::IMailReceiver *receiver_;
        std::unique_ptr<sharpen::IRemotePoster> poster_;
        std::atomic<sharpen::RemoteActorStatus> status_;
        std::unique_ptr<sharpen::IWorkerGroup> worker_;

        void DoPostShared(const sharpen::Mail *mail) noexcept;

        void DoPost(sharpen::Mail mail) noexcept;

        static void DoCancel(sharpen::Future<void> *future) noexcept;

        inline virtual std::uint64_t DoGetId() const noexcept override
        {
            assert(this->poster_);
            return this->poster_->GetId();
        }
    public:
    
        TcpActor(sharpen::IFiberScheduler &scheduler,sharpen::IMailReceiver &receiver,std::unique_ptr<sharpen::IRemotePoster> poster);
    
        virtual ~TcpActor() noexcept;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }
        
        virtual void Post(sharpen::Mail mail) override;

        virtual void PostShared(const sharpen::Mail &mail) override;

        inline virtual sharpen::RemoteActorStatus GetStatus() const noexcept override
        {
            return this->status_;
        }

        virtual void Cancel() noexcept override;
    };
}

#endif