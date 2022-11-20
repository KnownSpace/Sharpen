#pragma once
#ifndef _SHARPEN_WORKERGROUP_HPP
#define _SHARPEN_WORKERGROUP_HPP

#include <vector>

#include "AsyncBlockingQueue.hpp"
#include "NoexceptInvoke.hpp"
#include "IWorkerGroup.hpp"

namespace sharpen
{
    class EventEngine;

    class WorkerGroup:public sharpen::IWorkerGroup,public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Self = sharpen::WorkerGroup;

        void Entry(std::size_t index);

        virtual void DoSubmit(std::function<void()> task) override;

        std::atomic_bool token_;
        sharpen::AsyncBlockingQueue<std::function<void()>> queue_;
        std::vector<sharpen::AwaitableFuture<void>> workers_;
    public:
    
        explicit WorkerGroup(sharpen::EventEngine &engine);

        WorkerGroup(sharpen::EventEngine &engine,std::size_t workerCount);
    
        virtual ~WorkerGroup() noexcept
        {
            this->Stop();
            this->Join();
        }
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        virtual void Join() noexcept override;

        virtual void Stop() noexcept override;

        inline virtual bool Running() const noexcept override
        {
            return this->token_.load();
        }

        inline virtual std::size_t GetWorkerCount() const noexcept override
        {
            return this->workers_.size();
        }
    };   
}

#endif