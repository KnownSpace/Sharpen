#include <cstdio>
#include <iostream>
#include <sharpen/AwaitableFuture.hpp>
#include <sharpen/AsyncOps.hpp>
#include <sharpen/AsyncMutex.hpp>
#include <thread>
#include <ctime>
#include <mutex>
#include <string>
#include <sharpen/ThreadInfo.hpp>
#include <sharpen/FiberProcesserPool.hpp>
#include <sharpen/AsyncBarrier.hpp>
#include <sharpen/IFileChannel.hpp>
#include <sharpen/EventLoop.hpp>
#include <sharpen/Resumer.hpp>

#define TEST_COUNT 10000*100

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

void MutexTest(sharpen::AsyncMutex &lock,int &count)
{
    std::unique_lock<sharpen::AsyncMutex> _lock(lock);
    count++;
    std::printf("count is %d\n",count);
}

void ProcesserPoolTest()
{
    sharpen::FiberProcesserPool pool;
    sharpen::AsyncBarrier barrier(TEST_COUNT - 1);
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

void FileTest()
{
    sharpen::EventLoop loop(sharpen::MakeDefaultSelector());
    sharpen::FiberProcesser process([&loop]() mutable
    {
        sharpen::FileChannelPtr channel = sharpen::MakeFileChannel("./temp.txt",sharpen::FileAccessModel::All,sharpen::FileOpenModel::CreateOrOpen);
        channel->Register(&loop);
        const char content[] = "hello world\n";
        sharpen::ByteBuffer buf(content,sizeof(content));
        sharpen::Launch([]()
        {
            std::printf("writing\n");
        });
        sharpen::Size size = channel->WriteAsync(buf,0);
        std::printf("write %zd bytes complete\n",size);
        sharpen::Launch([](){
            std::printf("reading\n");
        });
        size = channel->ReadAsync(buf,0);
        std::string str(buf.Data(),size);
        std::printf("file content is:\n%s\n",str.c_str());
        loop.Stop();
    });
    loop.Run();
    process.Join();
}


int main(int argc, char const *argv[])
{
    std::printf("running in machine with %d cores\n",std::thread::hardware_concurrency());
    std::string arg("basic");
    if(argc > 1)
    {
        arg = argv[1];
    }
    //multithreaded await test
    if(arg == "basic")
    {
        sharpen::Size count = std::thread::hardware_concurrency();
        std::printf("test count is %zd\n",TEST_COUNT * count);
        std::vector<sharpen::FiberProcesser> vec;
        for (size_t i = 0; i < count; i++)
        {
            vec.push_back(std::move(sharpen::FiberProcesser(std::bind(&MultithreadAwaitTest))));
        }
        for (size_t i = 0; i < vec.size(); i++)
        {
            vec[i].Join();
        }
    }
    //mutex test
    if(arg == "mutex")
    {
        int count = 0;
        sharpen::AsyncMutex lock;
        sharpen::FiberProcesser t1(std::bind(&MutexTest,std::ref(lock),std::ref(count)));
        sharpen::FiberProcesser t2(std::bind(&MutexTest,std::ref(lock),std::ref(count)));
        t1.Join();
        t2.Join();
    }
    //workerpool test
    if (arg == "workerpool")
    {
        ProcesserPoolTest();
    }
    if (arg == "file")
    {
        FileTest();
    }
    std::printf("test complete\n");
    return 0;
}