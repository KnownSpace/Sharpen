#include <sharpen/IEventLoopGroup.hpp>
#include <sharpen/ITimer.hpp>
#include <sharpen/LinuxTimer.hpp>
#include <sharpen/WinTimer.hpp>

sharpen::TimerPtr sharpen::MakeTimer() {
    return sharpen::MakeTimer(sharpen::GetLocalLoopGroup());
}

sharpen::TimerPtr sharpen::MakeTimer(sharpen::EventLoop &loop) {
#ifdef SHARPEN_HAS_WAITABLETIMER
    (void)loop;
    sharpen::TimerPtr timer = std::make_shared<sharpen::WinTimer>();
    return timer;
#elif (defined SHARPEN_HAS_TIMERFD)
    std::shared_ptr<sharpen::LinuxTimer> timer = std::make_shared<sharpen::LinuxTimer>();
    timer->Register(loop);
    return timer;
#endif
}

sharpen::TimerPtr sharpen::MakeTimer(sharpen::IEventLoopGroup &loopGroup) {
    return sharpen::MakeTimer(loopGroup.RoundRobinLoop());
}