#pragma once
#ifndef _SHARPEN_EXECUTECONTEXT_HPP
#define _SHARPEN_EXECUTECONTEXT_HPP

#include <functional>

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_WIN

#define SHARPEN_HAS_FIBER 
#include <winbase.h>

#else

#define SHARPEN_HAS_UCONTEXT
#include <ucontext.h>

#endif


namespace sharpen
{
#ifdef SHARPEN_HAS_FIBER
    using NativeExecuteContextHandle = LPVOID;
#else
    using NativeExecuteContextHandle = ucontext_t;
#endif
  
    class ExecuteContext
    {
    private:
        using Self = sharpen::ExecuteContext;
        
        using Function = std::function<void()>;
    
        sharpen::NativeExecuteContextHandle handle_;
        
        static Self InternalMakeContext(Function *entry);
    public:
        
        ExecuteContext(sharpen::NativeExecuteContextHandle handle);
      
        void Switch();
        
        //should not be used directly
        static void InternalContextEntry(void *arg);
      
        template<typename _Fn,typename ..._Args>
        static Self MakeContext(_Fn &&fn,_Args &&...args)
        {
            Function *fn = new Function(std::bind(std::move(fn),args...));
            return Self::InternalMakeContext(fn);
        }
        
        static Self GetCurrentContext();
  };
}

#endif
