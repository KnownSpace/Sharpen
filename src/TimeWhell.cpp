#include <sharpen/TimeWheel.hpp>

#include <mutex>
#include <algorithm>

sharpen::TimeWheel::~TimeWheel() noexcept
{
    this->Stop();
}

void sharpen::TimeWheel::Tick()
{
    //collect timer
    TickBucket &bucket = this->buckets_[this->pos_ % this->buckets_.size()];
    std::list<TickCallback> cbs;
    {
        std::unique_lock<sharpen::SpinLock> lock(bucket.lock_);
        auto ite = bucket.cbs_.begin();
        while (ite != bucket.cbs_.end())
        {
            if (ite->round_ == 0)
            {
                cbs.push_back(std::move(*ite));
                ite = bucket.cbs_.erase(ite);
            }
            else
            {
                ite->round_ -= 1;
                ite++;
            }
        }
    }
    bool upstream{false};
    if (this->upstream_ && this->pos_ != 0 && (this->pos_ % this->buckets_.size()) == 0)
    {
        upstream = true;
    }
    this->pos_++;
    for (auto begin = cbs.begin(); begin != cbs.end(); begin++)
    {
        begin->cb_();
    }
    if (upstream)
    {
        this->upstream_->Tick();
    }
}