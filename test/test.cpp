#include <cstdio>
#include <iostream>
#include <sharpen/AwaitableFuture.hpp>
#include <sharpen/AsyncOps.hpp>
#include <sharpen/AsyncMutex.hpp>
#include <thread>
#include <ctime>
#include <mutex>
#include <string>
#include <sharpen/WorkerPool.hpp>

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

#define TEST_COUNT 1000*1000*1

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
    for(sharpen::Uint32 i = 0;i < TEST_COUNT;i++)
    {
        sharpen::AwaitableFuture<void> future;
        //std::printf("id: %u %u %p\n",sharpen::GetCurrentThreadId(),i,&future);
        sharpen::Launch([&future]()
        {
            
            future.Complete();
            //std::printf("id: %u future %p\n", sharpen::GetCurrentThreadId(), &future);
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

void MutexTest(sharpen::AsyncMutex &lock,int &count)
{
    std::unique_lock<sharpen::AsyncMutex> _lock(lock);
    count++;
    std::printf("count is %d\n",count);
}

void WorkerPoolTest()
{
    sharpen::WorkerPool pool(std::thread::hardware_concurrency());
    sharpen::AsyncBarrier barrier(TEST_COUNT);
    pool.Start();
    for (sharpen::Int32 i = 0; i < TEST_COUNT; ++i)
    {
        sharpen::Launch([i,&barrier]() {
            std::printf("Worker number %d count %d\n",sharpen::GetCurrentThreadId(),i);
            barrier.Notice();
        });
    }
    barrier.WaitAsync();
    pool.Stop();
}

int main(int argc, char const *argv[])
{
    std::printf("running in machine with %d cores\n",std::thread::hardware_concurrency());
    //multithreaded await test
    std::string arg("basic");
    if(argc > 1)
    {
        arg = argv[1];
    }
    if(arg == "basic")
    {
        std::printf("test count is %d\n",TEST_COUNT);
        std::vector<std::thread> vec;
        for (size_t i = 0; i < std::thread::hardware_concurrency(); i++)
        {
            vec.push_back(std::move(std::thread(std::bind(&MultithreadAwaitTest))));
        }
        for (size_t i = 0; i < vec.size(); i++)
        {
            vec[i].join();
        }
    }
    if(arg == "mutex")
    {
        int count = 0;
        sharpen::AsyncMutex lock;
        std::thread t1(std::bind(&MutexTest,std::ref(lock),std::ref(count))),t2(std::bind(&MutexTest,std::ref(lock),std::ref(count)));
        t1.join();
        t2.join();
    }
    if (arg == "workerpool")
    {
        WorkerPoolTest();
    }
    std::printf("test complete\n");
    return 0;
}
