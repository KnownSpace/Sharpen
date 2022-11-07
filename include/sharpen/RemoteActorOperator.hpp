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
    class RemoteActorOperator
    {
    private:
        using Self = sharpen::RemoteActorOperator;

        sharpen::IRemoteActor *actor_;
        sharpen::WorkerGroup worker_;
        bool isOpened_;

        void DoPost(sharpen::Future<bool> *future,const sharpen::IMail *mail) noexcept;
    public:
    
        explicit RemoteActorOperator(sharpen::EventEngine &engine,sharpen::IRemoteActor &actor);
    
        RemoteActorOperator(const Self &other);
    
        RemoteActorOperator(Self &&other) noexcept;
    
        inline Self &operator=(const Self &other)
        {
            if(this != std::addressof(other))
            {
                Self tmp{other};
                std::swap(tmp,*this);
            }
            return *this;
        }
    
        Self &operator=(Self &&other) noexcept;
    
        ~RemoteActorOperator() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        void PostAsync(sharpen::Future<bool> &future,const sharpen::IMail &mail);
    };   
}

#endif