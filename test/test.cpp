#include <cstdio>
#include <iostream>
#include <sharpen/AwaitableFuture.hpp>
#include <sharpen/AsyncOps.hpp>
#include <thread>
#include <ctime>

#ifdef SHARPEN_IS_WIN

namespace sharpen
{
    DWORD GetCurrentThreadId()
    {
        return ::GetCurrentThreadId();
    }
}

#else

#include <unistd.h>
#include <sys/syscall.h>

#define gettid() syscall(__NR_gettid)

namespace sharpen
{
    int GetCurrentThreadId()
    {
        return gettid();
    }
}

#endif

#define TEST_COUNT 1000000

void LaunchTest()
{
    for(size_t i = 0;i < TEST_COUNT;++i)
    {
        sharpen::Launch([](){
            //do nothing
        });
    }
}

void AwaitTest()
{
    for(size_t i = 0;i < TEST_COUNT;i++)
    {
        sharpen::AwaitableFuture<void> future;
        sharpen::Launch([&future](){
            future.Complete();
        });
        future.Await();
    }
}

void MultithreadAwaitTest()
{
    std::clock_t begin,end,time;
    begin = std::clock();
    AwaitTest();
    end = std::clock();
    time = (end-begin)/CLOCKS_PER_SEC;
    std::printf("AwaitTest using %d sec in thread %d\n",time,sharpen::GetCurrentThreadId());
}

void MultithreadLaunchTest()
{
    std::clock_t begin,end,time;
    begin = std::clock();
    LaunchTest();
    end = std::clock();
    time = (end-begin)/CLOCKS_PER_SEC;
    std::printf("LaunchTest using %d sec in thread %d\n",time,sharpen::GetCurrentThreadId());
}

int main(int argc, char const *argv[])
{
    std::printf("running in machine with %d cores\n",std::thread::hardware_concurrency());
    //multithreaded await test
    /*
    std::thread t1(std::bind(&MultithreadAwaitTest)),t2(std::bind(&MultithreadAwaitTest));
    t1.join();
    t2.join();
    */
    std::printf("test begin\n");
    std::unique_ptr<sharpen::ExecuteContext> octx(new sharpen::ExecuteContext());
    std::thread t([&octx]() mutable {
        auto ctx = std::move(sharpen::ExecuteContext::MakeContext([](){
            std::printf("do nothing\n");
            return;
        }));
        ctx->Switch(*octx);
        std::printf("success\n");
    });
    t.join();
    std::printf("begin switch\n");
    std::thread t1([&octx]() mutable
    {
        std::printf("switch\n");
        std::unique_ptr<sharpen::ExecuteContext> ctx(new sharpen::ExecuteContext());
        octx->Switch(*ctx);
        std::printf("never see\n");
    });
    t1.join();
    return 0;
}
