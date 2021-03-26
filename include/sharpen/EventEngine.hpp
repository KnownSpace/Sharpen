#pragma once
#ifndef _SHARPEN_EVENTENGINE_HPP
#define _SHARPEN_EVENTENGINE_HPP

#include "EventLoopThread.hpp"
#include "ISelector.hpp"

namespace sharpen
{
    class EventEngine
    {
    private:
        using Workers = std::vector<std::unique_ptr<sharpen::EventLoopThread>>;

        Workers workers_;
        sharpen::Size pos_;
    public:
        EventEngine();

        explicit EventEngine(sharpen::Size workerCount);
        
        ~EventEngine() noexcept;

        void Stop() noexcept;

        sharpen::EventLoop *RoundRobinLoop() noexcept;
    };
    
}

#endif