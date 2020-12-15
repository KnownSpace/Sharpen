#pragma once
#ifndef _SHARPEN_THREADPOOL_HPP
#define _SHARPEN_THREADPOOL_HPP

#include <functional>
#include <list>
#include <atomic>
#include <thread>

#include "AsyncSemaphore.hpp"
#include "SpinLock.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "AsyncBarrier.hpp"
#include "TypeDef.hpp"
#include "AwaitableFuture.hpp"

namespace sharpen
{
    class ThreadPool:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Task = std::function<void()>;
        using TaskList = std::list<MyTask>;
        
        sharpen::AsyncSemaphore flag_;
        sharpen::SpinLock lock_;
        TaskList tasks_;
        std::atomic_bool running_;
        sharpen::Size threadNum_;
        sharpen::AsyncBarrier stopFlag_;
    
        void ThreadEntry() noexcept;
    
    public:
        explicit ThreadPool(sharpen::Size threadNum);
        
        ~ThreadPool() noexcept;
        
        void Run();
        
        void Post(Task task);
        
        void Stop() noexcept;
    };
}

#endif
