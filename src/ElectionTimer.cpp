#include <sharpen/ElectionTimer.hpp>

void sharpen::ElectionTimer::WaitAsync(sharpen::Future<bool> &future)
{
    this->notify_ = &future;
    this->DoWaitAsync();
}

void sharpen::ElectionTimer::Reset()
{
    this->timer_->Cancel();
    this->future_.Reset();
    this->DoWaitAsync();
}

void sharpen::ElectionTimer::DoWaitAsync()
{
    if (!this->notify_)
    {
        return;
    }
    this->future_.SetCallback(std::bind(&sharpen::ElectionTimer::Notify,this,std::placeholders::_1));
    sharpen::Uint32 wait = (this->distrubution_)(this->random_);
    this->timer_->WaitAsync(this->future_,std::chrono::milliseconds(wait));
}

void sharpen::ElectionTimer::Notify(sharpen::Future<bool> &future)
{
    if(future.Get())
    {
        sharpen::Future<bool> *notify;
        std::swap(notify,this->notify_);
        if(notify)
        {
            notify->Complete(true);
        }
    }
}

void sharpen::ElectionTimer::Cancel()
{
    this->timer_->Cancel();
    sharpen::Future<bool> *notify;
    std::swap(notify,this->notify_);
    if(notify)
    {
        notify->Complete(false);
    }
}