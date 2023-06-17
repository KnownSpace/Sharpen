#include <sharpen/Fiber.hpp>
#include <cassert>
#include <stdexcept>

#include <sharpen/SystemError.hpp>

#ifdef __cplusplus
extern "C" {
#endif

fcontext_t make_fcontext(void *sp, size_t size, void (*fn)(transfer_t));
transfer_t jump_fcontext(fcontext_t const to, void *vp);
transfer_t ontop_fcontext(fcontext_t const to, void *vp, transfer_t (*fn)(transfer_t));

#ifdef __cplusplus
}
#endif

thread_local sharpen::FiberPtr sharpen::Fiber::currentFiber_;

std::atomic_uint64_t sharpen::Fiber::idAllocator_{0};

std::uint64_t sharpen::Fiber::AllocId() noexcept {
    return sharpen::Fiber::idAllocator_.fetch_add(1,std::memory_order_relaxed) + 1;
}

sharpen::Fiber::Fiber() noexcept
    : handle_(nullptr)
    , stack_()
    , task_()
    , callback_(nullptr)
    , inited_(false)
    , scheduler_(nullptr)
    , localStorage_()
    , id_(sharpen::Fiber::AllocId()) {
}

sharpen::Fiber::~Fiber() noexcept {
    this->Release();
}

std::uint64_t sharpen::Fiber::GetId() const noexcept {
    return this->id_;
}

void sharpen::Fiber::Release() noexcept {
    this->stack_.Release();
}

void sharpen::Fiber::Switch() noexcept {
    if (!this->inited_) {
        this->inited_ = true;
        this->InitFiber();
    }
    ::ontop_fcontext(this->handle_, this, &sharpen::Fiber::SaveCurrentAndSwitch);
}

void sharpen::Fiber::Switch(sharpen::Fiber *callback) noexcept {
    this->callback_ = callback;
    this->Switch();
}

transfer_t sharpen::Fiber::SaveCurrentAndSwitch(transfer_t from) noexcept {
    sharpen::Fiber::GetCurrentFiber()->handle_ = from.fctx;
    sharpen::Fiber *current = reinterpret_cast<sharpen::Fiber *>(from.data);
    sharpen::Fiber::currentFiber_ = current->shared_from_this();
    return from;
}

sharpen::FiberPtr sharpen::Fiber::GetCurrentFiber() noexcept {
    if (!sharpen::Fiber::currentFiber_) {
        sharpen::FiberPtr fiber = std::make_shared<sharpen::Fiber>();
        fiber->inited_ = true;
        sharpen::Fiber::currentFiber_ = std::move(fiber);
    }
    return sharpen::Fiber::currentFiber_;
}

sharpen::FiberLocalStorage &sharpen::Fiber::GetLocalStorage() noexcept {
    if (!sharpen::Fiber::currentFiber_) {
        sharpen::FiberPtr fiber = std::make_shared<sharpen::Fiber>();
        fiber->inited_ = true;
        sharpen::Fiber::currentFiber_ = std::move(fiber);
    }
    return sharpen::Fiber::currentFiber_->LocalStorage();
}

void sharpen::Fiber::FiberEntry(transfer_t from) noexcept {
    from = ::jump_fcontext(from.fctx, nullptr);
    sharpen::Fiber *fiber = reinterpret_cast<sharpen::Fiber *>(from.data);
    try {
        fiber->task_();
    } catch (const std::bad_alloc &fault) {
        (void)fault;
        std::terminate();
    } catch (const std::system_error &error) {
        if (sharpen::IsFatalError(error.code().value())) {
            std::terminate();
        }
    } catch (const std::exception &ignore) {
        assert(ignore.what() == nullptr && "an exception occured in event loop");
        (void)ignore;
    }
    sharpen::Fiber *cb{fiber->callback_};
    if (cb) {
        cb->Switch();
    }
}

void sharpen::Fiber::InitFiber() {
    sharpen::MemoryStack stack = sharpen::MemoryStack::AllocStack(this->stack_.GetSize());
    this->handle_ = ::make_fcontext(stack.Top(), stack.GetSize(), &sharpen::Fiber::FiberEntry);
    this->handle_ = ::jump_fcontext(this->handle_, nullptr).fctx;
    this->stack_ = std::move(stack);
}

sharpen::IFiberScheduler *sharpen::Fiber::GetScheduler() const noexcept {
    return this->scheduler_;
}

void sharpen::Fiber::SetScheduler(sharpen::IFiberScheduler *scheduler) noexcept {
    this->scheduler_ = scheduler;
}

std::uint64_t sharpen::Fiber::GetCurrentFiberId() noexcept {
    if (!currentFiber_) {
        return sharpen::Fiber::GetCurrentFiber()->GetId();
    }
    return currentFiber_->GetId();
}

sharpen::IFiberScheduler *sharpen::Fiber::GetCurrentFiberSceduler() noexcept {
    if (currentFiber_) {
        return currentFiber_->GetScheduler();
    }
    return nullptr;
}

sharpen::IFiberScheduler *sharpen::GetLocalSchedulerPtr() noexcept {
    return sharpen::Fiber::GetCurrentFiberSceduler();
}