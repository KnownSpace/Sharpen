#pragma once
#ifndef _SHARPEN_FIBERSCHEDULER_HPP
#define _SHARPEN_FIBERSCHEDULER_HPP

#include "Fiber.hpp"
#include "BlockingQueue.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"

namespace sharpen
{

    class FiberScheduler:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Func = std::function<void()>;

        static sharpen::BlockingQueue<sharpen::FiberPtr> fibers_;

        thread_local static sharpen::FiberPtr processer_;

        thread_local static std::unique_ptr<sharpen::FiberScheduler> scheduler_;

        bool running_;

        Func switchCallback_;
    public:
        FiberScheduler();

        ~FiberScheduler() noexcept;

        static bool IsProcesser() noexcept;

        void AsProcesser();

        template <class _Rep, class _Period>
        void ProcessOnce(const std::chrono::duration<_Rep, _Period> &timeout)
        {
            if (!this->running_)
            {
                return;
            }
            sharpen::FiberPtr fiber;
            bool success = fibers_.Pop(fiber,timeout);
            if (success && this->running_ && fiber)
            {
                sharpen::FiberPtr current = sharpen::Fiber::GetCurrentFiber();
                fiber->Switch(current);
            }
        }

        void Stop() noexcept;

        void Schedule(sharpen::FiberPtr &&fiber);

        static sharpen::FiberScheduler &GetScheduler();

        void SwitchToProcesser();

        void SwitchToProcesser(Func callback);
    };
    
}

#endif