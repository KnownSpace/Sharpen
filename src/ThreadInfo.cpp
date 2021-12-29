#include <sharpen/ThreadInfo.hpp>

#ifdef SHARPEN_IS_WIN

#include <Windows.h>
#elif defined SHARPEN_IS_LINUX
#include <unistd.h>
#include <sys/syscall.h>
#else
#include <pthread.h>
#endif

#include <sharpen/Option.hpp>

sharpen::Uint32 sharpen::GetCurrentThreadId() noexcept
{
    static thread_local sharpen::Option<sharpen::Uint32> id;
    if(!id.HasValue())
    {
#ifdef SHARPEN_IS_WIN
        id.Construct(::GetCurrentThreadId());
#elif defined SHARPEN_IS_LINUX
        id.Construct(syscall(__NR_gettid));
#else
        id.Construct(pthread_self());
#endif
    }
    return id.Get();
}