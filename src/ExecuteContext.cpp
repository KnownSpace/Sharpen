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

thread_local sharpen::ExecuteContextPtr sharpen::ExecuteContext::CurrentContext;

sharpen::ExecuteContext::ExecuteContext()
    :handle_()
    ,func_()
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

//error will be ignored
void sharpen::ExecuteContext::InternalDisableContextSwitch() noexcept
{
    if(sharpen::LocalEnableContextSwitch)
    {
#ifdef SHARPEN_HAS_FIBER
      ::ConvertFiberToThread();
#endif
      sharpen::LocalEnableContextSwitch = false;
    }
}

sharpen::ExecuteContext::~ExecuteContext() noexcept
{
#ifdef SHARPEN_HAS_FIBER
    if(this->handle_ != nullptr)
    {
      ::DeleteFiber(this->handle_);
    }
#else
    if(this->handle_.uc_stack.ss_sp != nullptr)
    {
        std::free(this->handle_.uc_stack.ss_sp);
        //::munmap(this->handle_.uc_stack.ss_sp,this->handle_.uc_stack.ss_size);
    }
#endif
}

void sharpen::ExecuteContext::Switch() noexcept
{
    assert(this != sharpen::ExecuteContext::GetCurrentContext().get());
#ifdef SHARPEN_HAS_FIBER
    assert(this->handle_ != nullptr);
    sharpen::ExecuteContext::CurrentContext = this->shared_from_this();
    ::SwitchToFiber(this->handle_);
#else
    sharpen::ExecuteContext *old = sharpen::ExecuteContext::GetCurrentContext().get();
    sharpen::ExecuteContext::CurrentContext = this->shared_from_this();
    ::swapcontext(&(old->handle_),&(this->handle_));
#endif
}

sharpen::ExecuteContextPtr sharpen::ExecuteContext::GetCurrentContext()
{
    if (sharpen::ExecuteContext::CurrentContext)
    {
        return sharpen::ExecuteContext::CurrentContext;
    }
    sharpen::ExecuteContextPtr ctx = std::make_shared<sharpen::ExecuteContext>();
#ifdef SHARPEN_HAS_FIBER
    sharpen::NativeExecuteContextHandle handle = GetCurrentFiber();
    ctx->handle_ = handle;
#else
    ::getcontext(&(ctx->handle_));
#endif
    sharpen::ExecuteContext::CurrentContext = ctx;
    return ctx;
}

void sharpen::ExecuteContext::InternalContextEntry(void *lpCtx)
{
    assert(lpCtx != nullptr);
    sharpen::ExecuteContext* ctx = (sharpen::ExecuteContext*)lpCtx;
    ctx->func_();
}

sharpen::ExecuteContextPtr sharpen::ExecuteContext::InternalMakeContext(sharpen::ExecuteContext::Function entry)
{
    assert(entry != nullptr);
    sharpen::ExecuteContextPtr ctx = std::make_shared<sharpen::ExecuteContext>();
    ctx->func_ = std::move(entry);
#ifdef SHARPEN_HAS_FIBER
    sharpen::NativeExecuteContextHandle handle = nullptr;
    handle = ::CreateFiberEx(4*1024,SHARPEN_CONTEXT_STACK_SIZE - 4*1024,FIBER_FLAG_FLOAT_SWITCH,(LPFIBER_START_ROUTINE)&sharpen::ExecuteContext::InternalContextEntry,ctx.get());
    if(handle == nullptr)
    {
        sharpen::ThrowLastError();
    }
    ctx->handle_ = handle;
#else
    ::getcontext(&(ctx->handle_));
    //ctx->handle_.uc_stack.ss_sp = ::mmap(nullptr,SHARPEN_CONTEXT_STACK_SIZE,PROT_READ|PROT_WRITE,MAP_PRIVATE | MAP_ANONYMOUS,-1,0);
    ctx->handle_.uc_stack.ss_sp = std::calloc(SHARPEN_CONTEXT_STACK_SIZE,sizeof(char));
    if(ctx->handle_.uc_stack.ss_sp == nullptr)
    {
        throw std::bad_alloc();
    }
    ctx->handle_.uc_stack.ss_size = SHARPEN_CONTEXT_STACK_SIZE;
    ctx->handle_.uc_link = nullptr;
    ::makecontext(&(ctx->handle_),(void(*)())&sharpen::ExecuteContext::InternalContextEntry,1, ctx.get());
#endif
    return ctx;
}
