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

    class ExecuteContext:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Self = sharpen::ExecuteContext;
        
        using Function = std::function<void()>;
    
        //it is a fiber(LPVOID) in windows
        //and a ucontext_t in *nix
        sharpen::NativeExecuteContextHandle handle_;
        
        //it is true if enable resource release
        bool enableAutoRelease_;
        
        static std::unique_ptr<Self> InternalMakeContext(Function *entry);
        
    public:
        ExecuteContext();
        
        //free the stack memory or delete fiber if necessary
        ~ExecuteContext() noexcept;
        
        void Switch() noexcept;
        
        void Switch(Self &oldContext) noexcept;
        
        //should not be used directly
        //lpFn is a pointer of std::function<void()>
        static void InternalContextEntry(void *lpFn);
      
        template<typename _Fn,typename ..._Args,typename = decltype(std::declval<_Fn>()(std::declval<_Args>()...))>
        static std::unique_ptr<Self> MakeContext(_Fn &&fn,_Args &&...args)
        {
            Function *func = new Function(std::bind(std::move(fn),args...));
            return Self::InternalMakeContext(func);
        }
        
        static std::unique_ptr<Self> GetCurrentContext();
        
        //it call ConvertThreadToFiberEx in windows and set sharpen::LocalEnableContextSwitch to true
        static void InternalEnableContextSwitch();
        
        //it call ConvertFiberToThread in windows and set sharpen::LocalEnableContextSwitch to false
        static void InternalDisableContextSwitch() noexcept;
        
        void SetAutoRelease(bool flag) noexcept;
  };
}

#endif
