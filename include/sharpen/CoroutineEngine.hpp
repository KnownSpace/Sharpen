#pragma once
#ifndef _SHARPEN_COROUTINEENGINE_HPP
#define _SHARPEN_COROUTINEENGINE_HPP

#include <memory>
#include <atomic>

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
    extern thread_local std::unique_ptr<sharpen::ExecuteContext> LocalEngineContext;

    //it save the last execute context and will be pushed into sharpen::CentralEngine when we switch to sharpen::LocalEngineContext
    extern thread_local std::unique_ptr<sharpen::ExecuteContext> LocalFromContext;
  
    //it is a internal class and you should never use it directly
    class CoroutineEngine:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using List = sharpen::BlockingQueue<sharpen::ExecuteContext>;
        
        List contexts_;
        
        std::atomic_bool alive_;

        void InternalPushTask(std::function<void()> &&fn);
        
        static void InitThisThread();
    public:
        
        //actually,we initializa LocalEngineContext in here
        void LoadEngineContext();
        
        //if there are no any context in the queue
        //we will block the thread
        sharpen::ExecuteContext WaitContext();
        
        //push a context to the queue
        void PushContext(sharpen::ExecuteContext &&context) noexcept;
        
        template<typename _Fn,typename ..._Args,typename = decltype(std::declval<_Fn>()(std::declval<_Args>()...))>
        void PushTask(_Fn &&fn,_Args &&...args)
        {
            std::function<void()> func = std::bind(std::move(fn),args...);
            this->InternalPushTask(std::move(func));
        }
        
        bool IsAlive() const;
    };
  
    extern sharpen::CoroutineEngine CentralEngine;
  
    //it is a internal function and you should not use it directly
    //equal to while(true) WaitContext();
    extern void CentralEngineLoopEntry(void *lpNull);
}

#endif
