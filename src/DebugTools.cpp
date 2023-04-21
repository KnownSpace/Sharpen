#include <sharpen/DebugTools.hpp>

#include <cstdarg>
#include <mutex>

#include <sharpen/AsyncMutex.hpp>

sharpen::AsyncMutex *sharpen::InternalSyncPrintMutex() noexcept
{
    static sharpen::AsyncMutex mutex;
    return &mutex;
}

int sharpen::SyncPrintf(const char *format, ...) noexcept
{
    sharpen::AsyncMutex *mutex{sharpen::InternalSyncPrintMutex()};
    std::unique_lock<sharpen::AsyncMutex> lock{*mutex};
    std::va_list args;
    va_start(args, format);
    int result{std::vprintf(format, args)};
    va_end(args);
    return result;
}

int sharpen::SyncPuts(const char *str) noexcept
{
    sharpen::AsyncMutex *mutex{sharpen::InternalSyncPrintMutex()};
    std::unique_lock<sharpen::AsyncMutex> lock{*mutex};
    return std::puts(str);
}