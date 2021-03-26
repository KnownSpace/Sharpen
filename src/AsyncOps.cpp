#include <sharpen/AsyncOps.hpp>

void sharpen::Delay()
{
    sharpen::FiberScheduler &scheduler = sharpen::FiberScheduler::GetScheduler();
    scheduler.ProcessOnce(std::chrono::milliseconds(100));
}