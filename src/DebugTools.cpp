#include <sharpen/DebugTools.hpp>

#include <sharpen/AsyncMutex.hpp>
#include <cstdarg>
#include <mutex>

sharpen::AsyncMutex *sharpen::InternalSyncPrintMutex() noexcept {
    static sharpen::AsyncMutex mutex;
    return &mutex;
}

int sharpen::SyncPrintf(const char *format, ...) noexcept {
    sharpen::AsyncMutex *mutex{sharpen::InternalSyncPrintMutex()};
    std::unique_lock<sharpen::AsyncMutex> lock{*mutex};
    std::va_list args;
    va_start(args, format);
    int result{std::vprintf(format, args)};
    va_end(args);
    return result;
}

int sharpen::SyncPuts(const char *str) noexcept {
    sharpen::AsyncMutex *mutex{sharpen::InternalSyncPrintMutex()};
    std::unique_lock<sharpen::AsyncMutex> lock{*mutex};
    return std::puts(str);
}

int sharpen::SyncDebugPrintf(const char *format, ...) noexcept {
#ifndef _NDEBUG
    sharpen::AsyncMutex *mutex{sharpen::InternalSyncPrintMutex()};
    std::unique_lock<sharpen::AsyncMutex> lock{*mutex};
    std::va_list args;
    va_start(args, format);
    int result{std::vprintf(format, args)};
    va_end(args);
    return result;
#endif
}

int sharpen::SyncDebugPuts(const char *str) noexcept {
#ifndef _NDEBUG
    return sharpen::SyncPuts(str);
#endif
}