#include <cstdio>
#include <cassert>
#include <sharpen/StopWatcher.hpp>
#include <sharpen/TimerOps.hpp>
#include <sharpen/EventEngine.hpp>

void TimerTest()
{
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    engine.Startup([&engine]()
    {
        std::printf("timer test begin\n");
        sharpen::TimerPtr timer = sharpen::MakeTimer(sharpen::EventEngine::GetEngine());
        std::printf("test cancel\n");
        sharpen::AwaitableFuture<bool> future;
        sharpen::StopWatcher sw;
        sw.Begin();
        timer->WaitAsync(future,std::chrono::seconds(3));
        timer->Cancel();
        future.Await();
        sw.Stop();
        assert(sw.Compute() < 3*CLOCKS_PER_SEC);
        std::printf("cancel using %zu tu,1 second = %zu tu\n",static_cast<std::size_t>(sw.Compute()),static_cast<size_t>(sw.TimeUnitPerSecond()));
        {
            sharpen::AwaitableFuture<void> future;
            sharpen::AwaitForResult result{sharpen::AwaitFor(future,timer,std::chrono::seconds(1))};
            assert(result == sharpen::AwaitForResult::Timeout);
        }
        {
            sharpen::AwaitableFuture<void> future;
            engine.Launch([&future]()
            {
                sharpen::Delay(std::chrono::milliseconds(500));
                future.Complete();
            });
            sharpen::AwaitForResult result{sharpen::AwaitFor(future,timer,std::chrono::seconds(1))};
            assert(result == sharpen::AwaitForResult::CompletedOrError);
        }
        std::printf("timer test pass\n");
    });
}

int main(int argc, char const *argv[])
{
    TimerTest();
    return 0;
}
