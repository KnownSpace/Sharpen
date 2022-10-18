#include <sharpen/TimerOps.hpp>

std::unique_ptr<sharpen::TimerPool> sharpen::TimerHelper::gobalTimerPool_{nullptr};

std::once_flag sharpen::TimerHelper::flag_;

void sharpen::TimerHelper::InitTimerPool(sharpen::EventEngine *engine,Maker maker)
{
    assert(engine != nullptr);
    auto *p = new sharpen::TimerPool{*engine,maker};
    if(!p)
    {
        throw std::bad_alloc();
    }
    gobalTimerPool_.reset(p);
}

sharpen::TimerPool &sharpen::TimerHelper::GetTimerPool(sharpen::EventEngine &engine,Maker maker)
{
    if(!sharpen::TimerHelper::gobalTimerPool_)
    {
        using FnPtr = void(*)(sharpen::EventEngine *,Maker);
        std::call_once(sharpen::TimerHelper::flag_,std::bind(static_cast<FnPtr>(&sharpen::TimerHelper::InitTimerPool),&engine,maker));
    }
    return *sharpen::TimerHelper::gobalTimerPool_;
}