#pragma once
#ifndef _SHARPEN_SINGLEWORKERGROUP_HPP
#define _SHARPEN_SINGLEWORKERGROUP_HPP

#include <functional>

#include "IWorkerGroup.hpp"
#include "AsyncBlockingQueue.hpp"
#include "NoexceptInvoke.hpp"
#include "IFiberScheduler.hpp"

namespace sharpen
{
    class SingleWorkerGroup:public sharpen::IWorkerGroup,public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Self = sharpen::SingleWorkerGroup;

        std::atomic_bool token_;
        sharpen::AsyncBlockingQueue<std::function<void()>> queue_;
        sharpen::AwaitableFuture<void> worker_;

        void Entry() noexcept;
    
        virtual void NviSubmit(std::function<void()> task) override;
    public:
    
        explicit SingleWorkerGroup(sharpen::IFiberScheduler &scheduler);
    
        virtual ~SingleWorkerGroup() noexcept;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        virtual void Stop() noexcept override;

        virtual void Join() noexcept override;

        virtual bool Running() const noexcept override;

        inline virtual std::size_t GetWorkerCount() const noexcept
        {
            return 1;
        }
    };
}

#endif