#pragma once
#ifndef _SHARPEN_COROUTINELOOP_HPP
#define _SHARPEN_COROUTINELOOP_HPP

#include "ExecuteContext.hpp"
#include "BlockingQueue.hpp"

namespace sharpen
{
    extern thread_local ExecuteContext *LoopContext;
    
    class CoroutineLoop;
    
    extern CoroutineLoop CentralCoroutineLoop;
}

#endif
