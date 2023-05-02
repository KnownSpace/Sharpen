#pragma once
#ifndef _SHARPEN_DEBUGTOOLS_HPP
#define _SHARPEN_DEBUGTOOLS_HPP

#include <cstdio>

namespace sharpen {
    class AsyncMutex;

    extern sharpen::AsyncMutex *InternalSyncPrintMutex() noexcept;

    extern int SyncPrintf(const char *format, ...) noexcept;

    extern int SyncPuts(const char *str) noexcept;
}   // namespace sharpen

#endif