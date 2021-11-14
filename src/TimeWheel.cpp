#include <sharpen/TimeWheel.hpp>

#include <sharpen/IteratorOps.hpp>

sharpen::TimeWheel::~TimeWheel() noexcept
{
    this->Stop();
}

void sharpen::TimeWheel::Tick()
{
    //collect timer
    TickBucket &bucket = this->buckets_[this->pos_ % this->buckets_.size()];
    using CompPtr = bool(*)(const TickCallback &,const TickCallback &);
    std::list<TickCallback> cbs;
    {
        std::unique_lock<sharpen::SpinLock> lock(bucket.lock_);
        auto begin = bucket.cbs_.begin(),end = bucket.cbs_.end();
        while (begin != end && begin->round_ == 0)
        {
            std::pop_heap(bucket.cbs_.begin(),bucket.cbs_.end(),std::bind(static_cast<CompPtr>(&Self::CompareRound),std::placeholders::_1,std::placeholders::_2));
            --end;
        }
        while (begin != end)
        {
            begin->round_ -= 1;
            ++begin;
        }
        begin = end;
        end = bucket.cbs_.end();
        cbs.assign(std::make_move_iterator(begin),std::make_move_iterator(end));
        bucket.cbs_.erase(begin,end);
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

void sharpen::TimeWheel::SetUpstream(sharpen::TimeWheelPtr upstream)
{
    if (!upstream)
    {
        this->upstream_.reset();
    }
    if (this->roundTime_ != upstream->waitTime_)
    {
        throw std::invalid_argument("upstream wait time != round time");
    }
    this->upstream_ = upstream;
}

void sharpen::TimeWheel::RunAsync()
{
    if (!this->timer_)
    {
        throw std::logic_error("this timer wheel is an upstream wheel");
    }
    this->running_ = true;
    while (this->running_)
    {
        this->timer_->Await(this->waitTime_);
        this->Tick();
    }
}