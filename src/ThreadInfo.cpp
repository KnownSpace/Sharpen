#include <sharpen/ThreadInfo.hpp>

#ifdef SHARPEN_IS_WIN

#include <Windows.h>

#else

#include <unistd.h>
#include <sys/syscall.h>

#endif


sharpen::Int32 sharpen::GetCurrentThreadId() noexcept
{
#ifdef SHARPEN_IS_WIN
        return ::GetCurrentThreadId();
#else
        return syscall(__NR_gettid);
#endif   
}