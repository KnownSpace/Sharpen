#include <cstring>
#include <cassert>
#include <stdexcept>

#include <sharpen/ExecuteContext.hpp>
#include <sharpen/TypeDef.hpp>
#include <sharpen/SystemError.hpp>

#ifdef SHARPEN_HAS_UCONTEXT
#include <sys/mman.h>
#endif

thread_local bool sharpen::LocalEnableContextSwitch(false);

sharpen::ExecuteContext::ExecuteContext()
    :handle_()
    ,enableAutoRelease_(false)
{
#ifdef SHARPEN_HAS_FIBER
    this->handle_ = nullptr;
#endif
}

void sharpen::ExecuteContext::InternalEnableContextSwitch()
{
    if(!sharpen::LocalEnableContextSwitch)
    {
#ifdef SHARPEN_HAS_FIBER
      LPVOID result = ::ConvertThreadToFiberEx(NULL,FIBER_FLAG_FLOAT_SWITCH);
      if(result == nullptr)
      {
          //throw a exception and stop program if we fail
          sharpen::ThrowLastError();
      }
#endif
      sharpen::LocalEnableContextSwitch = true;
    }
}

void sharpen::ExecuteContext::InternalDisableContextSwitch()
{
    if(sharpen::LocalEnableContextSwitch)
    {
#ifdef SHARPEN_HAS_FIBER
      ::ConvertFiberToThread();
#endif
      sharpen::LocalEnableContextSwitch = false;
    }
}

sharpen::ExecuteContext::~ExecuteContext()
{
#ifdef SHARPEN_HAS_FIBER
    if(this->enableAutoRelease_ && this->handle_ != nullptr)
    {
      ::DeleteFiber(this->handle_);
    }
#else
    if(this->enableAutoRelease_ && this->handle_.uc_stack.ss_sp != nullptr)
    {
        //use munmap to release memory
        ::munmap(this->handle_.uc_stack.ss_sp,this->handle_.uc_stack.ss_size);
    }
#endif
}

void sharpen::ExecuteContext::Switch()
{
#ifdef SHARPEN_HAS_FIBER
    assert(this->handle_ != nullptr);
    ::SwitchToFiber(this->handle_);
#else
    ::setcontext(&(this->handle_));
#endif
}

void sharpen::ExecuteContext::Switch(sharpen::ExecuteContext &oldContext)
{
#ifdef SHARPEN_HAS_FIBER
    oldContext.handle_ = GetCurrentFiber();
    this->Switch();
#else
    ::swapcontext(&(oldContext.handle_),&(this->handle_));
#endif
}

void sharpen::ExecuteContext::SetAutoRelease(bool flag)
{
    this->enableAutoRelease_ = flag;
}

std::unique_ptr<sharpen::ExecuteContext> sharpen::ExecuteContext::GetCurrentContext()
{
    std::unique_ptr<sharpen::ExecuteContext> ctx(new sharpen::ExecuteContext());
    if(!ctx)
    {
        throw std::bad_alloc();
    }
#ifdef SHARPEN_HAS_FIBER
    sharpen::NativeExecuteContextHandle handle = GetCurrentFiber();
    ctx->handle_ = handle;
#else
    ::getcontext(&(ctx->handle_));
#endif
    return std::move(ctx);
}

void sharpen::ExecuteContext::InternalContextEntry(void *lpFn)
{
    auto *p = (sharpen::ExecuteContext::Function*)lpFn;
    std::unique_ptr<sharpen::ExecuteContext::Function> fn(p);
    (*fn)();
}

std::unique_ptr<sharpen::ExecuteContext> sharpen::ExecuteContext::InternalMakeContext(sharpen::ExecuteContext::Function *entry)
{
    assert(entry != nullptr);
    std::unique_ptr<sharpen::ExecuteContext> ctx(new sharpen::ExecuteContext());
    if(!ctx)
    {
        throw std::bad_alloc();
    }
#ifdef SHARPEN_HAS_FIBER
    sharpen::NativeExecuteContextHandle handle = nullptr;
    handle = ::CreateFiberEx(SHARPEN_CONTEXT_STACK_SIZE,0,FIBER_FLAG_FLOAT_SWITCH,(LPFIBER_START_ROUTINE)&sharpen::ExecuteContext::InternalContextEntry,entry);
    if(handle == nullptr)
    {
        throw sharpen::ThrowLastError();
    }
    ctx->handle_ = handle;
#else
    ::getcontext(&(ctx->handle_));
    //use mmap to allocate statck
    ctx->handle_.uc_stack.ss_sp = ::mmap(nullptr,SHARPEN_CONTEXT_STACK_SIZE,PROT_READ|PROT_WRITE,MAP_PRIVATE | MAP_ANONYMOUS,-1,0);
    if(!ctx->handle_.uc_stack.ss_sp)
    {
        throw std::bad_alloc();
    }
    ctx->handle_.uc_stack.ss_size = SHARPEN_CONTEXT_STACK_SIZE;
    ctx->handle_.uc_link = nullptr;
    ::makecontext(&(ctx->handle_),(void(*)())&sharpen::ExecuteContext::InternalContextEntry,1,entry);
#endif
    return std::move(ctx);
}
