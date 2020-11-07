#pragma once
#ifndef _SHARPEN_COROUTINEWORKER_HPP
#define _SHARPEN_COROUTINEWORKER_HPP

#include <memory>

#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "ExecuteContext.hpp"
#include "CoroutineEngine.hpp"

namespace sharpen
{
    //coroutine engine worker 
    class CoroutineWorker:public sharpen::Noncopyable,public sharpen::Nonmoavble
    {
    private:
        using ContextPtr = std::unique_ptr<sharpen::ExecuteContext>;
        
        //save current execute context
        ContextPtr current_;
    public:
        CoroutineWorker();
        
        ~CoroutineWorker() noexcept;
        
        //block the thread and wait contexts to exceute
        void Run();
        
        //stop worker
        void Stop() noexcept;
    };
}

#endif
