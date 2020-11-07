#pragma once
#ifndef _SHARPEN_COROUTINEWORKER_HPP
#define _SHARPEN_COROUTINEWORKER_HPP

#include <memory>

#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "SpinLock.hpp"
#include "ExecuteContext.hpp"
#include "CoroutineEngine.hpp"

namespace sharpen
{
    //coroutine engine worker 
    class CoroutineWorker:public sharpen::Noncopyable,public sharpen::Nonmoavble
    {
    private:
        using ContextPtr = std::unique_ptr<sharpen::ExecuteContext>;
        
    public:
        CoroutineWorker() = default;
        
        ~CoroutineWorker() noexcept = default;
        
        //block the thread and wait contexts to exceute
        void Run();
    };
}

#endif
