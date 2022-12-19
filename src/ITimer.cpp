#include <sharpen/ITimer.hpp>
#include <sharpen/WinTimer.hpp>
#include <sharpen/LinuxTimer.hpp>
#include <sharpen/IEventLoopGroup.hpp>

sharpen::TimerPtr sharpen::MakeTimer(sharpen::EventLoop &loop)
{
#ifdef SHARPEN_HAS_WAITABLETIMER
    (void)loop;
    sharpen::TimerPtr timer = std::make_shared<sharpen::WinTimer>();
    return std::move(timer);
#elif (defined SHARPEN_HAS_TIMERFD)
    std::shared_ptr<sharpen::LinuxTimer> timer = std::make_shared<sharpen::LinuxTimer>();
    timer->Register(&loop);
    return timer;
#endif
}

sharpen::TimerPtr sharpen::MakeTimer(sharpen::IEventLoopGroup &loopGroup)
{
    return sharpen::MakeTimer(loopGroup.RoundRobinLoop());
}