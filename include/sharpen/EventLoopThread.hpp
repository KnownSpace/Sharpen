#pragma once
#ifndef _SHARPEN_EVENTLOOPTHREAD_HPP
#define _SHARPEN_EVENTLOOPTHREAD_HPP

#include <thread>

#include "EventLoop.hpp"

namespace sharpen
{
    class EventLoopThread:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:

        sharpen::EventLoop loop_;
        std::thread thread_;

        void Entry() noexcept;
    public:
        explicit EventLoopThread(sharpen::SelectorPtr selector);

        EventLoopThread(sharpen::SelectorPtr selector,std::shared_ptr<std::vector<std::function<void()>>> tasks,std::shared_ptr<sharpen::SpinLock> lock);

        ~EventLoopThread() noexcept;

        void Join();

        void Detach();

        void Stop() noexcept;

        sharpen::EventLoop *GetLoop() noexcept;
    };
    
}

#endif