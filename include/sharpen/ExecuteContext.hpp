#pragma once
#ifndef _SHARPEN_EXECUTECONTEXT_HPP
#define _SHARPEN_EXECUTECONTEXT_HPP

#include <functional>

#include "SystemMacro.hpp"
#include "TypeDef.hpp"
#include "Noncopyable.hpp"

#ifdef SHARPEN_IS_WIN

#define SHARPEN_HAS_FIBER 
//use fiber
#include <winbase.h>
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
     
    class ExecuteContext:public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::ExecuteContext;
        
        using Function = std::function<void()>;
    
        //it is a fiber(LPVOID) in windows
        //and a ucontext_t in *nix
        sharpen::NativeExecuteContextHandle handle_;
        
#ifdef SHARPEN_HAS_UCONTEXT
        //set this flag if has been moved
        bool moved_;
#endif
        
        static Self InternalMakeContext(Function *entry);
        
        explicit ExecuteContext(sharpen::NativeExecuteContextHandle handle);
        
    public:
        ExecuteContext(Self &&other) noexcept;
        
        Self &operator=(Self &&other) noexcept;
      
        //free the stack memory or delete fiber if necessary
        ~ExecuteContext() noexcept;
        
        void Switch();
        
        //should not be used directly
        //lpFn is a pointer of std::function<void()>
        static void InternalContextEntry(void *lpFn);
      
        template<typename _Fn,typename ..._Args>
        static Self MakeContext(_Fn &&fn,_Args &&...args)
        {
            Function *fn = new Function(std::bind(std::move(fn),args...));
            return Self::InternalMakeContext(fn);
        }
        
        static Self GetCurrentContext();
        
        //it call ConvertThreadToFiberEx in windows and set sharpen::LocalEnableContextSwitch to true
        static void InternalEnableContextSwitch();
        
        //it call ConvertFiberToThread in windows and set sharpen::LocalEnableContextSwitch to false
        static void InteralDisableContextSwitch();
  };
}

#endif
