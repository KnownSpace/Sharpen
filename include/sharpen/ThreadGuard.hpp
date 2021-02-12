#pragma once
#ifndef _SHARPEN_EXITWATCHDOG_HPP
#define _SHARPEN_EXITWATCHDOG_HPP

#include "Nonmovable.hpp"
#include "ExecuteContext.hpp"

namespace sharpen
{
    //it is a internal class and you should not use it directly
    class ThreadGuard:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
    public:
        ThreadGuard() = default;
        
        //we will free resource in here
        ~ThreadGuard() noexcept;

        void ReleaseResource() noexcept;
    };
    
    //it release coroutine resource when thread exit
    //we will do nothing if sharpen::LocalSchedulerContext is nullptr
    extern thread_local ThreadGuard LocalThreadGuard;
}

#endif
