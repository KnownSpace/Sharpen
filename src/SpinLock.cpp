#include <sharpen/SpinLock.hpp>

#include <thread>

sharpen::SpinLock::SpinLock()
    :flag_()
{}

void sharpen::SpinLock::lock()
{
    while (this->flag_.test_and_set(std::memory_order::memory_order_acquire))
    {}
}

void sharpen::SpinLock::unlock() noexcept
{
    this->flag_.clear(std::memory_order::memory_order_release);
}