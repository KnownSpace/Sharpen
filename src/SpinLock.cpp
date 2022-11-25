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

void sharpen::SpinLock::lock() noexcept
{
    std::size_t i{0};
    while (this->flag_.test_and_set())
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
    this->flag_.clear();
}

bool sharpen::SpinLock::TryLock()
{
    return !this->flag_.test_and_set();
}

#ifdef SHARPEN_PAUSE
#undef SHARPEN_PAUSE
#endif