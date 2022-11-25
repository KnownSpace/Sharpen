#include <sharpen/YieldOps.hpp>

void sharpen::YieldCycle()
{
    sharpen::FiberPtr current = sharpen::Fiber::GetCurrentFiber();
    sharpen::IFiberScheduler *scheduler = current->GetScheduler();
    //this thread is not a processer
    if(!scheduler || !scheduler->IsProcesser())
    {
        std::this_thread::yield();
    }
    //this thread is a processer
    else
    {
        scheduler->SetSwitchCallback(sharpen::YieldCycleCallback{std::move(current),scheduler});
        scheduler->SwitchToProcesserFiber();
    }
}