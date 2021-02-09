#pragma once
#ifndef _SHARPEN_COROUTINEENGINE_HPP
#define _SHARPEN_COROUTINEENGINE_HPP

#include <memory>
#include <atomic>
#include <functional>

#include "ExecuteContext.hpp"
#include "Nonmovable.hpp"
#include "BlockingQueue.hpp"

namespace sharpen
{
    //one engine context per thread
    //this pointer is nullptr by default
    //if you use a coroutine function like Await()
    //we will make some initializational operations
    //it will call ConvertThreadToFiberEx and CreateFiberEx in windows or call getcontext and makecontext in *nix
    extern thread_local sharpen::ExecuteContextPtr LocalEngineContext;

    extern thread_local std::function<void()> LocalContextSwitchCallback;
  
    //it is a internal class and you should never use it directly
    class CoroutineEngine:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using ContextPtr = sharpen::ExecuteContextPtr;
        
        using List = sharpen::BlockingQueue<ContextPtr>;
        
        List contexts_;

        void InternalPushTask(std::function<void()> fn);
    
    public:
        
        CoroutineEngine();
        
        ~CoroutineEngine() noexcept = default;
        
        //if there are no any context in the queue
        //we will block the thread
        ContextPtr WaitContext() noexcept;
        
        //push a context to the queue
        void PushContext(ContextPtr context);
        
        template<typename _Fn,typename ..._Args,typename = decltype(std::declval<_Fn>()(std::declval<_Args>()...))>
        void PushTask(_Fn &&fn,_Args &&...args)
        {
            std::function<void()> func = std::bind(std::forward<_Fn>(fn),std::forward<_Args>(args)...);
            this->InternalPushTask(std::move(func));
        }
   
    };
  
    extern sharpen::CoroutineEngine CentralEngine;
  
    //it is a internal function and you should not use it directly
    //equal to while(true) WaitContext();
    extern void CentralEngineLoopEntry();
    
    extern void InitThisThreadForCentralEngine();
}

#endif
