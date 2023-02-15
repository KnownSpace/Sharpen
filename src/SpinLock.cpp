#include <sharpen/SpinLock.hpp>

#include <thread>

#include <sharpen/CompilerInfo.hpp>
#include <cstddef>
#include <cstdint>

#ifdef SHARPEN_COMPILER_MSVC
#define SHARPEN_PAUSE
#else
#if (defined __x86_64__ || defined __x86_64 || defined __x86__ || defined __x86)
#define SHARPEN_PAUSE asm("pause")
#else
#define SHARPEN_PAUSE
#endif
#endif

sharpen::SpinLock::SpinLock() noexcept
    :acquireCount_(0)
    ,releaseCount_(0)
{}

void sharpen::SpinLock::lock() noexcept
{
    std::size_t i{0};
    std::uint64_t count{this->acquireCount_.fetch_add(1,std::memory_order::memory_order_acq_rel)};
    while (count != this->releaseCount_.load(std::memory_order::memory_order_acquire))
    {
        SHARPEN_PAUSE;
        ++i;
        if(i == 256)
        {
            std::this_thread::yield();
            i = 0;
        }
    }
}

void sharpen::SpinLock::unlock() noexcept
{
    this->releaseCount_.fetch_add(1,std::memory_order::memory_order_acq_rel);
}

bool sharpen::SpinLock::TryLock() noexcept
{
    std::uint64_t count{this->acquireCount_.load(std::memory_order::memory_order_acquire)};
    std::uint64_t expectedCount{count};
    count += 1;
    return this->acquireCount_.compare_exchange_strong(expectedCount,count,std::memory_order::memory_order_acq_rel);
}

#ifdef SHARPEN_PAUSE
#undef SHARPEN_PAUSE
#endif