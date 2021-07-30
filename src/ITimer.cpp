#include <sharpen/ITimer.hpp>

#include <sharpen/AwaitableFuture.hpp>

void sharpen::ITimer::Await()
{
    sharpen::AwaitableFuture<void> future;
    this->WaitAsync(future);
    future.Await();
}