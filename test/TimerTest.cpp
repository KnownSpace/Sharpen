#include <cstdio>
#include <sharpen/TimeWheel.hpp>
#include <sharpen/StopWatcher.hpp>
#include <cassert>

void TimeWheelTest()
{
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    engine.Startup([]()
    {
        std::printf("timer test begin\n");
        sharpen::TimerPtr timer = sharpen::MakeTimer(sharpen::EventEngine::GetEngine());
        sharpen::TimeWheel wheel(std::chrono::seconds(1),10,timer);
        sharpen::TimeWheelPtr upstream = std::make_shared<sharpen::TimeWheel>(std::chrono::seconds(10),6);
        bool token = false;
        upstream->Put(std::chrono::seconds(10),[&wheel,&token]() mutable
        {
            assert(token);
            wheel.Stop();
        });
        wheel.SetUpstream(upstream);
        wheel.Put(std::chrono::seconds(9),[&wheel,&token]() mutable
        {
            std::printf("set token true\n");
            token = true;
        });
        wheel.RunAsync();
        std::printf("test cancel\n");
        sharpen::AwaitableFuture<bool> future;
        sharpen::StopWatcher sw;
        sw.Begin();
        timer->WaitAsync(future,std::chrono::seconds(3));
        timer->Cancel();
        future.Await();
        sw.Stop();
        assert(sw.Compute() < 3*CLOCKS_PER_SEC);
        std::printf("cancel using %zu tu,1 second = %zu tu\n",static_cast<sharpen::Size>(sw.Compute()),static_cast<size_t>(CLOCKS_PER_SEC));
        std::printf("timer test pass\n");
    });
}

int main(int argc, char const *argv[])
{
    TimeWheelTest();
    return 0;
}
