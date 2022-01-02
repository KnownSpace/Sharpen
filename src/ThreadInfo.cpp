#include <sharpen/ThreadInfo.hpp>

#ifdef SHARPEN_IS_WIN

#include <Windows.h>
#elif defined SHARPEN_IS_LINUX
#include <unistd.h>
#include <sys/syscall.h>
#else
#include <pthread.h>
#endif

#include <sharpen/Optional.hpp>

sharpen::Uint32 sharpen::GetCurrentThreadId() noexcept
{
    static thread_local sharpen::Optional<sharpen::Uint32> id;
    if(!id.Exist())
    {
#ifdef SHARPEN_IS_WIN
        id.Construct(::GetCurrentThreadId());
#elif defined SHARPEN_IS_LINUX
        id.Construct(static_cast<sharpen::Uint32>(syscall(__NR_gettid)));
#else
        id.Construct(pthread_self());
#endif
    }
    return id.Get();
}