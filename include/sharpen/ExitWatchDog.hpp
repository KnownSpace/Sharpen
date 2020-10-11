#pragma once
#ifndef _SHARPEN_EXITWATCHDOG_HPP
#define _SHARPEN_EXITWATCHDOG_HPP

#include "Nonmovable.hpp"
#include "ExecuteContext.hpp"

namespace sharpen
{
    //it is a internal class and you should not use it directly
    class ExitWatchDog:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
    public:
        ExitWatchDog() = default;
        
        //we will free resource in here
        ~ExitWatchDog() noexcept;

        void ReleaseResource() noexcept();
    };
    
    //it release coroutine resource when thread exit
    //we will do nothing if sharpen::LocalEngineContext is nullptr
    extern thread_local ExitWatchDog LocalWatchDog;
}

#endif
