#include <cstdio>
#include <cassert>

#include <sharpen/AsyncOps.hpp>
#include <sharpen/AwaitOps.hpp>
#include <sharpen/WorkerGroup.hpp>
#include <sharpen/TimerOps.hpp>
#include <sharpen/AsyncNagleBarrier.hpp>

void AwaitTest()
{
    std::puts("await test");
    auto f1 = sharpen::Async([]()
    {
        std::fputs("hello ",stdout);
        return 1;
    });
    auto f2 = sharpen::Async([](){
        std::puts("world");
    });
    auto f3 = sharpen::Async([](){
        return "hello world";
    });
    int r;
    std::tie(r,std::ignore,std::ignore) = sharpen::AwaitAll(*f1,*f2,*f3);
    assert(r == 1);
    std::printf("ret %d\n",r);
    auto f4 = sharpen::Async([]()
    {
        sharpen::Delay(std::chrono::seconds(3));
        std::puts("ok1");
    });
    auto f5 = sharpen::Async([](){
        sharpen::Delay(std::chrono::seconds(1));
        std::puts("ok2");
    });
    sharpen::AwaitAny(*f5,*f4);
    f4->Await();
    f5->Await();
    std::puts("await test pass");
    std::puts("reset test begin");
    sharpen::AwaitableFuture<int> future;
    sharpen::Launch([&future](){
        future.Complete(2);
    });
    r = future.Await();
    assert(r == 2);
    future.Reset();
    sharpen::Launch([&future](){
        future.Complete(3);
    });
    r = future.Await();
    assert(r == 3);
    std::puts("reset test pass");
    std::puts("worker group test begin");
    sharpen::WorkerGroup workers{sharpen::EventEngine::GetEngine()};
    auto workerFuture = workers.Invoke([]()
    {
        std::puts("work");
        return 0;
    });
    r = workerFuture->Await();
    assert(r == 0);
    std::puts("worker group test pass");
    std::puts("nagle test begin");
    {
        sharpen::AsyncNagleBarrier barrier{sharpen::GetGobalTimerPool().GetTimer(),std::chrono::seconds(3),10};
        std::puts("nagle wait");
        std::size_t count{barrier.WaitAsync()};
        std::printf("nagle wait done %zu\n",count);
        assert(count == 0);
    }
    {
        sharpen::AsyncNagleBarrier barrier{sharpen::GetGobalTimerPool().GetTimer(),std::chrono::seconds(3),10};
        std::puts("nagle wait");
        for (size_t i = 0; i != 7; ++i)
        {
            sharpen::Launch([&barrier]()
            {
                sharpen::Delay(std::chrono::seconds(1));
                barrier.NotifyOnce();
            });   
        }
        std::size_t count{barrier.WaitAsync()};
        std::printf("nagle wait done %zu\n",count);
        assert(count == 7);
    }
    {
        sharpen::AsyncNagleBarrier barrier{sharpen::GetGobalTimerPool().GetTimer(),std::chrono::seconds(3),10};
        std::puts("nagle wait");
        for (size_t i = 0; i != 10; ++i)
        {
            sharpen::Launch([&barrier]()
            {
                sharpen::Delay(std::chrono::seconds(1));
                barrier.NotifyOnce();
            });   
        }
        std::size_t count{barrier.WaitAsync()};
        std::printf("nagle wait done %zu\n",count);
        assert(count == 10);
    }
    std::puts("nagle test pass");
}

int main()
{
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    engine.Startup(&AwaitTest);
    return 0;
}
