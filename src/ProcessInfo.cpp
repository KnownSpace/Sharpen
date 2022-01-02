#include <sharpen/ProcessInfo.hpp>

#ifdef SHARPEN_IS_WIN
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include <sharpen/Optional.hpp>

sharpen::Uint32 sharpen::GetProcessId() noexcept
{
    static sharpen::Optional<sharpen::Uint32> id;
    if(!id.Exist())
    {
#ifdef SHARPEN_IS_WIN
        id.Construct(::GetCurrentProcessId());
#else
        id.Construct(static_cast<sharpen::Uint32>(::getpid()));
#endif
    }
    return id.Get();
}