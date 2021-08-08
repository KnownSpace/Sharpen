#pragma once
#ifndef _SHARPEN_TIMEWHEEL_HPP
#define _SHARPEN_TIMEWHEEL_HPP

#include <vector>
#include <atomic>
#include <memory>
#include <list>
#include <mutex>

#include "ITimer.hpp"
#include "SpinLock.hpp"

namespace sharpen
{
    class TimeWheel;

    using TimeWheelPtr = std::shared_ptr<sharpen::TimeWheel>;

    class TimeWheel:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:

        struct TickCallback
        {
        private:
            using Cb = std::function<void()>;
        public:
            sharpen::Size round_;
            Cb cb_;
        };
        

        struct TickBucket
        {
        private:
            using Cb = typename sharpen::TimeWheel::TickCallback;
            using Cbs = std::list<Cb>;
        public:
            sharpen::SpinLock lock_;
            Cbs cbs_;
        };

        std::chrono::milliseconds waitTime_;
        sharpen::TimeWheelPtr upstream_;
        std::vector<typename sharpen::TimeWheel::TickBucket> buckets_;
        sharpen::Size pos_;
        std::atomic_bool running_;
        std::chrono::milliseconds roundTime_;
        sharpen::TimerPtr timer_;

        void Tick();
    public:
        template<typename _Rep,typename _Period>
        TimeWheel(const std::chrono::duration<_Rep,_Period> &duration,sharpen::Size count,sharpen::TimerPtr timer)
            :waitTime_(duration)
            ,upstream_(nullptr)
            ,buckets_(count)
            ,pos_(0)
            ,running_(false)
            ,roundTime_(duration * count)
            ,timer_(timer)
        {}

        template<typename _Rep,typename _Period>
        TimeWheel(const std::chrono::duration<_Rep,_Period> &duration,sharpen::Size count)
            :TimeWheel(duration,count,nullptr)
        {}

        template<typename _Rep,typename _Period>
        void Put(const std::chrono::duration<_Rep,_Period> &duration,std::function<void()> task)
        {
            sharpen::TimeWheel::TickCallback cb;
            cb.cb_ = std::move(task);
            cb.round_ = duration / this->roundTime_;
            sharpen::Size buck = (duration % this->roundTime_) / this->waitTime_;
            buck += this->pos_;
            buck %= this->buckets_.size();
            buck -= 1;
            auto &bucket = this->buckets_[buck];
            {
                std::unique_lock<sharpen::SpinLock> lock(bucket.lock_);
                bucket.cbs_.push_back(std::move(cb));
            }
        }

        inline void SetUpstream(sharpen::TimeWheelPtr upstream)
        {
            if (this->roundTime_ != upstream->waitTime_)
            {
                throw std::invalid_argument("upstream wait time != round time");
            }
            this->upstream_ = upstream;
        }

        inline void RunAsync()
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

        inline void Stop()
        {
            this->running_ = false;
        }

        ~TimeWheel() noexcept;
    };
}

#endif