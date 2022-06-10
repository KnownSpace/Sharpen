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
        std::size_t pos_;
        std::unique_ptr<sharpen::EventLoop> mainLoop_;
        std::vector<sharpen::EventLoop*> loops_;

        static thread_local SwitchCallback switchCb_;

        static void ProcessFiber(sharpen::FiberPtr fiber);
        
        EventEngine();

        explicit EventEngine(std::size_t workerCount);

        static void CallSwitchCallback();

        void ProcessStartup(std::function<void()> fn);

        void ProcessStartupWithCode(std::function<int()> fn,int *code);
    public:
        
        virtual ~EventEngine() noexcept;

        static Self &SetupEngine();

        static Self &SetupEngine(std::size_t workerCount);

        static Self &SetupSingleThreadEngine();

        static Self &GetEngine();

        void Stop() noexcept;

        sharpen::EventLoop *RoundRobinLoop() noexcept;

        virtual void Schedule(sharpen::FiberPtr &&fiber) override;

        virtual bool IsProcesser() const override;

        virtual void SwitchToProcesserFiber() noexcept override;

        virtual void SetSwitchCallback(std::function<void()> fn) override;

        void Run();

        template<typename _Fn,typename ..._Args,typename _Check = sharpen::EnableIf<sharpen::IsCompletedBindableReturned<void,_Fn,_Args...>::Value>>
        void Startup(_Fn &&fn,_Args &&...args)
        {
            std::function<void()> task{std::bind(std::forward<_Fn>(fn),std::forward<_Args>(args)...)};
            this->Launch(&Self::ProcessStartup,this,task);
            this->Run();
        }

        template<typename _Fn,typename ..._Args,typename _Check = sharpen::EnableIf<sharpen::IsCompletedBindableReturned<int,_Fn,_Args...>::Value>>
        int StartupWithCode(_Fn &&fn,_Args &&...args)
        {
            int code{0};
            std::function<int()> task{std::bind(std::forward<_Fn>(fn),std::forward<_Args>(args)...)};
            this->Launch(&Self::ProcessStartupWithCode,this,task,&code);
            this->Run();
            return code;
        }

        std::size_t LoopCount() const noexcept
        {
            return this->loops_.size();
        }
    };
}

#endif