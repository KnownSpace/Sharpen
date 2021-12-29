#include <sharpen/ProcessInfo.hpp>

#ifdef SHARPEN_IS_WIN
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include <sharpen/Option.hpp>

sharpen::Uint32 sharpen::GetProcessId() noexcept
{
    static sharpen::Option<sharpen::Uint32> id;
    if(!id.HasValue())
    {
#ifdef SHARPEN_IS_WIN
        id.Construct(::GetCurrentProcessId());
#else
        id.Construct(static_cast<sharpen::Uint32>(::getpid()));
#endif
    }
    return id.Get();
}