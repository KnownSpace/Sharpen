#include <sharpen/ProcessInfo.hpp>

#ifdef SHARPEN_IS_WIN
#include <Windows.h>
#else
#include <unistd.h>
#endif

sharpen::Uint32 sharpen::GetProcessId() noexcept
{
#ifdef SHARPEN_IS_WIN
    return ::GetCurrentProcessId();
#else
    return ::getpid();
#endif
}