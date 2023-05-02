#include <sharpen/ThreadInfo.hpp>

#include <sharpen/Optional.hpp>

#ifdef SHARPEN_IS_WIN

#include <Windows.h>
#elif defined SHARPEN_IS_LINUX
#include <sys/syscall.h>
#include <unistd.h>
#else
#include <pthread.h>
#endif

std::uint32_t sharpen::GetCurrentThreadId() noexcept {
    static thread_local sharpen::Optional<std::uint32_t> id;
    if (!id.Exist()) {
#ifdef SHARPEN_IS_WIN
        id.Construct(::GetCurrentThreadId());
#elif defined SHARPEN_IS_LINUX
        id.Construct(static_cast<std::uint32_t>(syscall(__NR_gettid)));
#else
        id.Construct(pthread_self());
#endif
    }
    return id.Get();
}