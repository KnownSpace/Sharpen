#include <cstdlib>
#include <cassert>

#include <sharpen/ExecuteContext.hpp>

thread_local bool sharpen::LocalEnableContextSwitch(false);

sharpen::ExecuteContext::ExecuteContext()
    :handle_()
    ,enableAutoRelease_(false)
{}

void sharpen::ExecuteContext::InternalEnableContextSwitch()
{
    if(!sharpen::LocalEnableContextSwitch)
    {
#ifdef SHARPEN_HAS_FIBER
      ::ConvertThreadToFiberEx(NULL,FIBER_FLAG_FLOAT_SWITCH);
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
    if(this->enableAutoRelease_ && this->handle_.uc_stack.ss_up != nullptr)
    {
      std::free(this->handle_.uc_stack.ss_up);
    }
#endif
}

void sharpen::ExecuteContext::Switch()
{
#ifdef SHARPEN_HAS_FIBER
    std::assert(this->handle_ != nullptr);
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

std::unique_ptr<sharpen::ExecuteContext> sharpen::GetCurrentContext()
{
    std::unique_ptr ctx(new sharpen::ExecuteContext());
#ifdef SHARPEN_HAS_FIBER
    sharpen::NativeExecuteContxtHandle handle = GetCurrentFiber();
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
    (*p)();
}
