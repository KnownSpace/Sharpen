#include <sharpen/ProcessInfo.hpp>

#ifdef SHARPEN_IS_WIN
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include <sharpen/Optional.hpp>

std::uint32_t sharpen::GetProcessId() noexcept
{
    static sharpen::Optional<std::uint32_t> id;
    if (!id.Exist())
    {
#ifdef SHARPEN_IS_WIN
        id.Construct(::GetCurrentProcessId());
#else
        id.Construct(static_cast<std::uint32_t>(::getpid()));
#endif
    }
    return id.Get();
}