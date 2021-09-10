#pragma once
#ifndef _SHARPEN_EVENTENGINE_HPP
#define _SHARPEN_EVENTENGINE_HPP

#include <mutex>

#include "EventLoopThread.hpp"
#include "ISelector.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "IFiberScheduler.hpp"
#include "TypeTraits.hpp"

namespace sharpen
{
    class EventEngine:public sharpen::IFiberScheduler
    {
    private:
        using Workers = std::vector<std::unique_ptr<sharpen::EventLoopThread>>;
        using Self = sharpen::EventEngine;
        using SelfPtr = std::unique_ptr<Self>;
        using SwitchCallback = std::function<void()>;

        static SelfPtr engine_;
        static std::once_flag flag_;

        Workers workers_;
        sharpen::Size pos_;
        std::unique_ptr<sharpen::EventLoop> mainLoop_;
        std::vector<sharpen::EventLoop*> loops_;

        static thread_local SwitchCallback switchCb_;

        static void ProcessFiber(sharpen::FiberPtr fiber);
        
        EventEngine();

        explicit EventEngine(sharpen::Size workerCount);

        static void CallSwitchCallback();
    public:
        
        virtual ~EventEngine() noexcept;

        static Self &SetupEngine();

        static Self &SetupEngine(sharpen::Size workerCount);

        static Self &SetupSingleThreadEngine();

        static Self &GetEngine();

        void Stop() noexcept;

        sharpen::EventLoop *RoundRobinLoop() noexcept;

        virtual void Schedule(sharpen::FiberPtr &&fiber) override;

        virtual bool IsProcesser() const override;

        virtual void SwitchToProcesserFiber() noexcept override;

        virtual void SetSwitchCallback(std::function<void()> fn) override;

        void Run();

        template<typename _Fn,typename ..._Args,typename _Check = sharpen::EnableIf<sharpen::IsCallable<_Fn,_Args...>::Value>>
        void LaunchAndRun(_Fn &&fn,_Args &&...args)
        {
            this->Launch(std::forward<_Fn>(fn),std::forward<_Args>(args)...);
            this->Run();
        }

        template<typename _Fn,typename ..._Args,typename _Check = sharpen::EnableIf<sharpen::IsCallable<_Fn,_Args...>::Value>>
        void Startup(_Fn &&fn,_Args &&...args)
        {
            auto task = std::bind(std::forward<_Fn>(fn),std::forward<_Args>(args)...);
            this->Launch([task,this]() mutable
            {
                try
                {
                    task();
                }
                catch(const std::exception& ignore)
                {
                    (void)ignore;
                }
                this->Stop();
            });
            this->Run();
        }

        sharpen::Size LoopNumber() const noexcept
        {
            return this->loops_.size();
        }
    };
}

#endif