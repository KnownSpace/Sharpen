#include <sharpen/YieldOps.hpp>

void sharpen::YieldCycle() {
    sharpen::FiberPtr current = sharpen::Fiber::GetCurrentFiber();
    sharpen::IFiberScheduler *scheduler = current->GetScheduler();
    // this thread is not a processer
    if (!scheduler || !scheduler->IsProcesser()) {
        std::this_thread::yield();
    } else {
        // this thread is a processer
        scheduler->SetSwitchCallback(sharpen::YieldCycleCallback{std::move(current)});
        scheduler->SwitchToProcesserFiber();
    }
}

constexpr static std::size_t BusyLoopCount{16};

void sharpen::YieldCycleForBusyLoop() {
    for(std::size_t i = 0;i != BusyLoopCount;++i)
    {
        sharpen::YieldCycle();
    }
}