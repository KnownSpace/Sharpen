#include <cstdio>
#include <iostream>
#include <sharpen/AwaitableFuture.hpp>
#include <sharpen/AsyncOps.hpp>
#include <thread>

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

int main(int argc, char const *argv[])
{
    sharpen::AwaitableFuture<int> future;
    std::thread t([&future]() mutable
    {
        std::printf("i am %d and waiting for future\n",sharpen::GetCurrentThreadId());
        int r = future.Await();
        std::printf("result is %d\n",r);
    });
    sharpen::Launch([&future]() mutable
    {
        std::printf("i am %d and please input result:\n",sharpen::GetCurrentThreadId());
        int r;
        std::cin >> r;
        future.Complete(r);
    });
    t.join();
    return 0;
}
