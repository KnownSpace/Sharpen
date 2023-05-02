#pragma once
#ifndef _SHARPEN_EVENTLOOPTHREAD_HPP
#define _SHARPEN_EVENTLOOPTHREAD_HPP

#include "EventLoop.hpp"
#include <thread>

namespace sharpen {
    class EventLoopThread
        : public sharpen::Noncopyable
        , public sharpen::Nonmovable {
    private:
        sharpen::EventLoop loop_;
        std::thread thread_;

        void Entry() noexcept;

    public:
        explicit EventLoopThread(sharpen::SelectorPtr selector);

        ~EventLoopThread() noexcept;

        void Join();

        void Detach();

        void Stop() noexcept;

        sharpen::EventLoop *GetLoop() noexcept;
    };

}   // namespace sharpen

#endif