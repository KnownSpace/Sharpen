#pragma once
#ifndef _SHARPEN_REMOTEACTOROPERATOR_HPP
#define _SHARPEN_REMOTEACTOROPERATOR_HPP

#include <utility>

#include "AwaitableFuture.hpp"
#include "AsyncBlockingQueue.hpp"
#include "IRemoteActor.hpp"
#include "WorkerGroup.hpp"

namespace sharpen
{
    class IRemoteActorProposer:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Self = sharpen::IRemoteActorProposer;

        sharpen::IRemoteActor *actor_;
        std::unique_ptr<sharpen::WorkerGroup> worker_;
        bool isOpened_;

        void DoPropose(sharpen::Future<bool> *future,const sharpen::IMail *mail) noexcept;

    protected:

        virtual bool OnResponse(const sharpen::IMail &mail) noexcept = 0;
    public:
    
        IRemoteActorProposer(std::unique_ptr<sharpen::WorkerGroup> singleWorker,sharpen::IRemoteActor &actor) noexcept;
    
        virtual ~IRemoteActorProposer() noexcept
        {
            this->Cancel();
        }
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        void ProposeAsync(sharpen::Future<bool> &future,const sharpen::IMail &mail);

        void Cancel() noexcept;
    };   
}

#endif