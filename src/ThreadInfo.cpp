#include <sharpen/ThreadInfo.hpp>

#ifdef SHARPEN_IS_WIN

#include <Windows.h>
#elif defined SHARPEN_IS_LINUX
#include <unistd.h>
#include <sys/syscall.h>
#else
#include <pthread.h>
#endif


sharpen::Uint32 sharpen::GetCurrentThreadId() noexcept
{
#ifdef SHARPEN_IS_WIN
        return ::GetCurrentThreadId();
#elif defined SHARPEN_IS_LINUX
        return syscall(__NR_gettid);
#else
        return pthread_self();
#endif   
}