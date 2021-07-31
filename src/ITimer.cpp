#include <sharpen/ITimer.hpp>
#include <sharpen/WinTimer.hpp>
#include <sharpen/LinuxTimer.hpp>
#include <sharpen/EventEngine.hpp>

sharpen::TimerPtr sharpen::MakeTimer(sharpen::EventLoop &loop)
{
#ifdef SHARPEN_HAS_WAITABLETIMER
    sharpen::TimerPtr timer = std::make_shared<sharpen::WinTimer>();
    return std::move(timer);
#elif (defined SHARPEN_HAS_TIMERFD)
    std::shared_ptr<sharpen::LinuxTimer> timer = std::make_shared<sharpen::LinuxTimer>();
    timer->Register(&loop);
    return std::move(timer);
#endif
}

sharpen::TimerPtr sharpen::MakeTimer(sharpen::EventEngine &engine)
{
    return sharpen::MakeTimer(*engine.RoundRobinLoop());
}