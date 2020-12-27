#pragma once
#ifndef _SHARPEN_WORKERPOOL_HPP
#define _SHARPEN_WORKERPOOL_HPP

#include <thread>

#include "AsyncSemaphore.hpp"
#include "AsyncBarrier.hpp"

namespace sharpen
{
    class WorkerPool:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        
        //running
        bool running_;
        //worker count
        sharpen::Size count_;
        //stop flag
        sharpen::AsyncBarrier stopFlag_;
        //thread flag
        sharpen::AsyncSemaphore threadFlag_;
    public:
    
        explicit WorkerPool(sharpen::Size count);
        
        ~WorkerPool() noexcept;
        
        //start working
        void Start();
        
        //stop all worker
        void Stop() noexcept;
        
        //get worker count
        sharpen::Size GetWorkerCount() const noexcept;
        
        //set worker count
        void SetWorkerCount(sharpen::Size count);
    };
}

#endif
