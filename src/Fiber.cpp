#include <sharpen/Fiber.hpp>
#include <cassert>

thread_local sharpen::FiberPtr sharpen::Fiber::currentFiber_;

sharpen::Fiber::Fiber() noexcept
    :handle_(nullptr)
    ,stack_()
    ,task_()
    ,callback_()
    ,inited_(false)
{}

sharpen::Fiber::~Fiber() noexcept
{
    this->Release();
}

void sharpen::Fiber::Release() noexcept
{
    this->stack_.Release();
}

void sharpen::Fiber::Switch()
{
    if (!this->inited_)
    {
        this->inited_ = true;
        this->InitFiber();
    }
    ::ontop_fcontext(this->handle_,this,&sharpen::Fiber::SaveCurrentAndSwitch);
}

void sharpen::Fiber::Switch(const sharpen::FiberPtr &callback)
{
    this->callback_ = callback;
    this->Switch();
}

transfer_t sharpen::Fiber::SaveCurrentAndSwitch(transfer_t from)
{
    sharpen::Fiber::GetCurrentFiber()->handle_ = from.fctx;
    sharpen::Fiber *current = reinterpret_cast<sharpen::Fiber*>(from.data);
    sharpen::Fiber::currentFiber_ = current->shared_from_this();
    return from;
}

sharpen::FiberPtr sharpen::Fiber::GetCurrentFiber()
{
    if (!sharpen::Fiber::currentFiber_)
    {
        sharpen::FiberPtr fiber = std::make_shared<sharpen::Fiber>();
        fiber->inited_ = true;
        sharpen::Fiber::currentFiber_ = fiber;
    }
    return sharpen::Fiber::currentFiber_;
}

void sharpen::Fiber::FiberEntry(transfer_t from)
{
    from = ::jump_fcontext(from.fctx,nullptr);
    sharpen::Fiber *fiber = reinterpret_cast<sharpen::Fiber*>(from.data);
    try
    {
        fiber->task_();
    }
    catch(const std::exception& ignore)
    {
        assert(ignore.what() != nullptr);
        (void)ignore;
    }
    if (!fiber->callback_.expired())
    {
        fiber->callback_.lock()->Switch();
    }
}

void sharpen::Fiber::InitFiber()
{
    sharpen::MemoryStack stack = sharpen::MemoryStack::AllocStack(this->stack_.Size());
    this->handle_ = ::make_fcontext(stack.Top(),stack.Size(),&sharpen::Fiber::FiberEntry);
    this->handle_ = ::jump_fcontext(this->handle_,nullptr).fctx;
    this->stack_ = std::move(stack);
}

sharpen::FiberPtr sharpen::Fiber::InternalMakeFiber(sharpen::Size stackSize,sharpen::Fiber::Task task)
{
    if (stackSize == 0)
    {
        return nullptr;
    }
    sharpen::FiberPtr fiber = std::make_shared<sharpen::Fiber>();
    sharpen::MemoryStack stack = sharpen::MemoryStack::AllocStack(stackSize);
    fiber->handle_ = ::make_fcontext(stack.Top(),stack.Size(),&sharpen::Fiber::FiberEntry);
    fiber->handle_ = ::jump_fcontext(fiber->handle_,nullptr).fctx;
    fiber->task_ = std::move(task);
    fiber->stack_ = std::move(stack);
    return std::move(fiber);
}