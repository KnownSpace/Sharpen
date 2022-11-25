#pragma once
#ifndef _SHARPEN_DYNAMICWORKERGROUP_HPP
#define _SHARPEN_DYNAMICWORKERGROUP_HPP

#include <vector>
#include <functional>

#include "IWorkerGroup.hpp"
#include "NoexceptInvoke.hpp"
#include "AsyncBlockingQueue.hpp"
#include "IFiberScheduler.hpp"

namespace sharpen
{

    class DynamicWorkerGroup:public sharpen::IWorkerGroup,public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Self = sharpen::DynamicWorkerGroup;
    
        constexpr static std::size_t probeCount_{2};

        sharpen::IFiberScheduler *scheduler_;
        std::atomic_bool token_;
        std::size_t busyMark_;
        mutable sharpen::SpinLock lock_;
        std::atomic_size_t taskCount_;
        std::vector<std::unique_ptr<sharpen::AwaitableFuture<void>>> workers_;
        sharpen::AsyncBlockingQueue<std::function<void()>> queue_;

        bool MoreWorker() const noexcept;

        void CreateWorker();

        void Entry(sharpen::AwaitableFuture<void> *future) noexcept;
    
        virtual void DoSubmit(std::function<void()> task) override;
    public:
    
        constexpr static std::size_t defaultBusyMark_{128};

        explicit DynamicWorkerGroup(sharpen::IFiberScheduler &scheduler);

        DynamicWorkerGroup(sharpen::IFiberScheduler &scheduler,std::size_t workerCount);

        DynamicWorkerGroup(sharpen::IFiberScheduler &scheduler,std::size_t workerCount,std::size_t busyMark);
    
        virtual ~DynamicWorkerGroup() noexcept;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        virtual void Stop() noexcept override;

        virtual void Join() noexcept override;

        inline virtual bool Running() const noexcept override
        {
            return this->token_;
        }

        virtual std::size_t GetWorkerCount() const noexcept override;
    };   
}

#endif