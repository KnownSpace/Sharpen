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
  
    class ExecuteContext:public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::ExecuteContext;
        
        using Function = std::function<void()>;
    
        sharpen::NativeExecuteContextHandle handle_;
        
#ifdef SHARPEN_HAS_UCONTEXT
        //the stack may be in the heap
        sharpen::Char *stack_;
        
        bool isStackMemory_;
#endif
        
        static Self InternalMakeContext(Function *entry);
    public:
        
        explicit ExecuteContext(sharpen::NativeExecuteContextHandle handle);
        
        ExecuteContext(Self &&other) noexcept;
        
        Self &operator=(Self &&other) noexcept;
      
        //free the stack memory if necessary
        ~ExecuteContext() noexcept;
        
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
