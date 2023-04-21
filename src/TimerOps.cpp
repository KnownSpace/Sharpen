#include <sharpen/TimerOps.hpp>

#include <new>

std::unique_ptr<sharpen::TimerPool> sharpen::TimerHelper::gobalTimerPool_{nullptr};

std::once_flag sharpen::TimerHelper::flag_;

void sharpen::TimerHelper::InitTimerPool(sharpen::IEventLoopGroup *loopGroup, Maker maker)
{
    assert(loopGroup != nullptr);
    auto *p = new (std::nothrow) sharpen::TimerPool{*loopGroup, maker};
    if (!p)
    {
        throw std::bad_alloc();
    }
    gobalTimerPool_.reset(p);
}

sharpen::TimerPool &sharpen::TimerHelper::GetTimerPool(sharpen::IEventLoopGroup &loopGroup,
                                                       Maker maker)
{
    if (!sharpen::TimerHelper::gobalTimerPool_)
    {
        using FnPtr = void (*)(sharpen::IEventLoopGroup *, Maker);
        std::call_once(
            sharpen::TimerHelper::flag_,
            std::bind(static_cast<FnPtr>(&sharpen::TimerHelper::InitTimerPool), &loopGroup, maker));
    }
    return *sharpen::TimerHelper::gobalTimerPool_;
}