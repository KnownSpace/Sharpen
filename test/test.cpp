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
    for (size_t i = 0; i < 50; i++)
    {
        sharpen::SharedAwaitableFuturePtr<int> future = sharpen::MakeSharedAwaitableFuture<int>();
        std::thread t([future]()
        {
            int r = future->Await();
            std::printf("result is %d\n",r);
        });
        t.detach();
        future->Complete(i);
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(3));
    return 0;
}
