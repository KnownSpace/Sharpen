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

        void DoPost(const sharpen::Mail *mail) noexcept;
    public:
    
        TcpActor(sharpen::IFiberScheduler &scheduler,sharpen::IMailReceiver &receiver,std::unique_ptr<sharpen::IRemotePoster> poster);
    
        ~TcpActor() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline const sharpen::IRemotePoster &Poster() const noexcept
        {
            assert(this->poster_);
            return *this->poster_;
        }

        inline sharpen::IMailReceiver &MailReceiver() noexcept
        {
            assert(this->receiver_);
            return *this->receiver_;
        }
        
        inline const sharpen::IMailReceiver &MailReceiver() const noexcept
        {
            assert(this->receiver_);
            return *this->receiver_;
        }

        virtual void Post(const sharpen::Mail &mail) override;

        inline virtual sharpen::RemoteActorStatus GetStatus() const noexcept override
        {
            return this->status_;
        }

        virtual void Cancel() noexcept override;
    };
}

#endif