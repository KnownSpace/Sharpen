#include <sharpen/RandomTimerAdaptor.hpp>
#include <cstdio>

void sharpen::RandomTimerAdaptor::WaitAsync(sharpen::Future<bool> &future)
{
    sharpen::Uint32 waitMs = (this->distrubution_)(this->random_);
    this->timer_->WaitAsync(future,waitMs);
}

void sharpen::RandomTimerAdaptor::Cancel()
{
    this->timer_->Cancel();
}