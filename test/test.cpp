#include <cstdio>
#include <iostream>
#include <stdint.h>
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
#include <sharpen/EventEngine.hpp>
#include <sharpen/INetStreamChannel.hpp>
#include <sharpen/IpEndPoint.hpp>
#include <algorithm>

#define TEST_COUNT 10000 * 100

void AwaitTest()
{
    for (sharpen::Uint32 i = 0; i < TEST_COUNT; i++)
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
    std::clock_t begin, end, time;
    begin = std::clock();
    AwaitTest();
    end = std::clock();
    time = (end - begin) / CLOCKS_PER_SEC;
    std::printf("AwaitTest using %d sec in thread %d\n", time, sharpen::GetCurrentThreadId());
}

void MutexTest(sharpen::AsyncMutex &lock, int &count)
{
    std::unique_lock<sharpen::AsyncMutex> _lock(lock);
    count++;
    std::printf("count is %d\n", count);
}

void ProcesserPoolTest()
{
    sharpen::FiberProcesserPool pool;
    sharpen::AsyncBarrier barrier(TEST_COUNT - 1);
    for (sharpen::Int32 i = 0; i < TEST_COUNT; ++i)
    {
        sharpen::Launch([i, &barrier]()
        {
            std::printf("Worker number %d count %d\n", sharpen::GetCurrentThreadId(), i);
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
        sharpen::FileChannelPtr channel = sharpen::MakeFileChannel("./temp.txt", sharpen::FileAccessModel::All, sharpen::FileOpenModel::CreateOrOpen);
        channel->Register(&loop);
        const char content[] = "hello world\n";
        sharpen::ByteBuffer buf(content, sizeof(content) - 1);
        sharpen::Launch([](){ std::printf("writing\n"); });
        sharpen::Size size = channel->WriteAsync(buf, 0);
        std::printf("write %zd bytes complete\n", size);
        sharpen::Launch([](){ std::printf("reading\n"); });
        size = channel->ReadAsync(buf, 0);
        std::string str(buf.Data(), size);
        std::printf("file content is:\n%s\n", str.c_str());
        loop.Stop();
    });
    loop.Run();
    process.Join();
}

void HandleClient(sharpen::NetStreamChannelPtr client, std::string &close)
{
    bool keepalive = true;
    sharpen::ByteBuffer buf(4096);
    thread_local static sharpen::Uint32 count{0};
    while (keepalive)
    {
        try
        {
            sharpen::Size size = client->ReadAsync(buf);
            if (size == 0)
            {
                keepalive = false;
                break;
            }
            const char data[] = "HTTP/1.1 200\r\nConnection: keep-alive\r\nContent-Length: 2\r\n\r\nOK";
            client->WriteAsync(data, sizeof(data) - 1);
            if (std::search(buf.Begin(), buf.End(), close.begin(), close.end()) != buf.End())
            {
                keepalive = false;
            }
            count++;
            //std::printf("new data %d\n",count);
        }
        catch (const std::exception &e)
        {
            keepalive = false;
            break;
        }
    }
}

void WebTest()
{
    sharpen::StartupNetSupport();
    sharpen::FiberProcesserPool pool;
    sharpen::EventEngine engine;
    sharpen::NetStreamChannelPtr server = sharpen::MakeTcpStreamChannel(sharpen::AddressFamily::Ip);
    sharpen::IpEndPoint addr;
    addr.SetAddr("127.0.0.1");
    addr.SetPort(8080);
    server->SetReuseAddress(true);
    server->Bind(addr);
    server->Listen(65535);
    server->Register(engine);
    std::string close("close");
    sharpen::Launch([&server, &close, &engine]()
    {
        while (true)
        {
            sharpen::NetStreamChannelPtr client = server->AcceptAsync();
            client->Register(engine);
            sharpen::Launch(&HandleClient, client, close);
        }
    });
    std::cin.get();
    sharpen::CleanupNetSupport();
}

void NetTest()
{
    sharpen::StartupNetSupport();
    sharpen::EventLoop loop(sharpen::MakeDefaultSelector());
    sharpen::FiberProcesser processer([&loop]() mutable
    {
        ::printf("open channel\n");
        sharpen::IpEndPoint addr(0, 25565);
        addr.SetAddr("127.0.0.1");
        sharpen::NetStreamChannelPtr ser = sharpen::MakeTcpStreamChannel(sharpen::AddressFamily::Ip);
        ser->SetReuseAddress(true);
        ser->Bind(addr);
        ser->Register(&loop);
        ::printf("binding\n");
        ::printf("listening\n");
        ser->Listen(65535);
        sharpen::NetStreamChannelPtr clt = sharpen::MakeTcpStreamChannel(sharpen::AddressFamily::Ip);
        addr.SetPort(0);
        clt->Bind(addr);
        clt->Register(&loop);
        sharpen::Launch([&ser, &loop]() mutable
        {
            try
            {
                std::printf("accepting\n");
                sharpen::NetStreamChannelPtr clt = ser->AcceptAsync();
                sharpen::IpEndPoint addr;
                clt->GetRemoteEndPoint(addr);
                std::string ip(20, 0);
                addr.GetAddr(const_cast<char *>(ip.data()), 20);
                std::printf("new connection: ip %s port %d \n", ip.c_str(), addr.GetPort());
                clt->Register(&loop);
                char str[] = "Hello World\n";
                std::printf("writing\n");
                clt->WriteAsync(str, sizeof(str));
                std::printf("write completely\n");
            }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << '\n';
            }
        });
        try
        {
            addr.SetPort(25565);
            std::printf("conecting\n");
            clt->ConnectAsync(addr);
            sharpen::ByteBuffer buf(64);
            std::printf("reading\n");
            clt->ReadAsync(buf);
            std::cout << buf.Data() << "\n";
        }
            catch (const std::exception &e)
            {
                std::cerr << e.what() << '\n';
            }

            loop.Stop();
        });
    loop.Run();
    processer.Join();
    sharpen::CleanupNetSupport();
}

int main(int argc, char const *argv[])
{
    std::printf("running in machine with %d cores\n", std::thread::hardware_concurrency());
    std::string arg("basic");
    if (argc > 1)
    {
        arg = argv[1];
    }
    //multithreaded await test
    if (arg == "basic")
    {
        sharpen::Size count = std::thread::hardware_concurrency();
        std::printf("test count is %zd\n", TEST_COUNT * count);
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
    if (arg == "mutex")
    {
        int count = 0;
        sharpen::AsyncMutex lock;
        sharpen::FiberProcesser t1(std::bind(&MutexTest, std::ref(lock), std::ref(count)));
        sharpen::FiberProcesser t2(std::bind(&MutexTest, std::ref(lock), std::ref(count)));
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
    if (arg == "net")
    {
        NetTest();
    }
    if (arg == "web")
    {
        WebTest();
    }
    std::printf("test complete\n");
    return 0;
}