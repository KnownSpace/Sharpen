#pragma once
#ifndef _SHARPEN_COROUTINEWORKERPOOL_HPP
#define _SHARPEN_COROUTINEWORKERPOOL_HPP

#include <thread>
#include <vector>
#include <memory>

#include "TypeDef.hpp"
#include "CoroutineWorker.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"

namespace sharpen
{
    class CoroutineWorkerPool:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using ThreadPtr = std::unique_ptr<std::thread>;
        using WorkerPtr = std::unique_ptr<sharpen::CoroutineWorker>;
        using ThreadVector = std::vector<ThreadPtr>;
        using WorkerVector = std::vector<WorkerPtr>;
        using Lock = sharpen::SpinLock;
        
        //WorkerVector workers_;
        //ThreadVector threads_;
        sharpen::Size size_;
        Lock lock_;
        bool running_;
    public:
        explicit CoroutineWorkerPool(sharpen::Size poolSize);
        
        ~CoroutineWorkerPool() noexcept = default;
        
        void Run();
    };
}

#endif
