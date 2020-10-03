#include <sharpen/SpinLock.hpp>

#include <thread>

sharpen::SpinLock::SpinLock()
    :flag_(false)
{}

void sharpen::SpinLock::lock()
{
    bool store = false;
    while (!this->flag_.compare_exchange_weak(store,true))
    {
        store = false;
        std::this_thread::yield();
    }
}

void sharpen::SpinLock::unlock() noexcept
{
    this->flag_.store(false);
}