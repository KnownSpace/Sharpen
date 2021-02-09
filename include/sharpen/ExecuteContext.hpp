#pragma once
#ifndef _SHARPEN_EXECUTECONTEXT_HPP
#define _SHARPEN_EXECUTECONTEXT_HPP

//the stack size of execute context
#ifndef SHARPEN_CONTEXT_STACK_SIZE
//64kb by default
#define SHARPEN_CONTEXT_STACK_SIZE 64*1024
#endif

#include <type_traits>
#include <functional>
#include <memory>

#include "SystemMacro.hpp"
#include "TypeDef.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"

#ifdef SHARPEN_IS_WIN
#define SHARPEN_HAS_FIBER 
//use fiber
#include <Windows.h>
#else
#define SHARPEN_HAS_UCONTEXT
//use ucontext
#include <ucontext.h>
#endif


namespace sharpen
{
#ifdef SHARPEN_HAS_FIBER
    using NativeExecuteContextHandle = LPVOID;
#else
    using NativeExecuteContextHandle = ucontext_t;
#endif
    
    //it is true if current thread enable context switch function
    extern thread_local bool LocalEnableContextSwitch;

    class ExecuteContext;

    using ExecuteContextPtr = std::shared_ptr<sharpen::ExecuteContext>;

    //should never be used by directly
    //use sharpen::AwaitableFuture
    class ExecuteContext:public sharpen::Noncopyable,public sharpen::Nonmovable,public std::enable_shared_from_this<sharpen::ExecuteContext>
    {
    private:
        using Self = sharpen::ExecuteContext;
        
        using Function = std::function<void()>;
    
        //it is a fiber(LPVOID) in windows
        //and a ucontext_t in *nix
        sharpen::NativeExecuteContextHandle handle_;

        Function func_;
         
        static sharpen::ExecuteContextPtr InternalMakeContext(Function entry);

        static thread_local sharpen::ExecuteContextPtr CurrentContext;
        
    public:
        ExecuteContext();
        
        //free the stack memory or delete fiber
        ~ExecuteContext() noexcept;
        
        //before invoking it please save current context by yourself
        void Switch() noexcept;
        
        //should not be used directly
        //lpFn is a pointer to sharpen::ExecuteContext
        static void InternalContextEntry(void *lpCtx);
      
        template<typename _Fn,typename ..._Args,typename = decltype(std::declval<_Fn>()(std::declval<_Args>()...))>
        static sharpen::ExecuteContextPtr MakeContext(_Fn &&fn,_Args &&...args)
        {
            auto&& ctx = Self::InternalMakeContext(std::bind(std::forward<_Fn>(fn), std::forward<_Args>(args)...));
            return std::move(ctx);
        }
        
        static sharpen::ExecuteContextPtr GetCurrentContext();
        
        //it call ConvertThreadToFiberEx in windows and set sharpen::LocalEnableContextSwitch to true
        static void InternalEnableContextSwitch();
        
        //it call ConvertFiberToThread in windows and set sharpen::LocalEnableContextSwitch to false
        static void InternalDisableContextSwitch() noexcept;
  };
}

#endif
