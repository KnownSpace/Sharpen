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
        
        static ExecuteContext InternalMakeContext(Function *entry);
    public:
      
        void Switch();
      
        template<typename _Fn,typename ..._Args>
        static ExecuteContext MakeContext(_Fn &&fn,_Args &&...args);
        
  };
}

#endif
