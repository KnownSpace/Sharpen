#pragma once
#ifndef _SHARPEN_ELECTION_TIMER
#define _SHARPEN_ELECTION_TIMER

#include <random>
#include <chrono>

#include "ITimer.hpp"
#include "AwaitableFuture.hpp"

namespace sharpen
{
    class RandomTimerAdaptor
    {
    private:
        using Self = sharpen::RandomTimerAdaptor;

        sharpen::TimerPtr timer_;
        std::minstd_rand random_;
        std::uniform_int_distribution<sharpen::Uint32> distrubution_;
    public:
        template<typename _Rep1,typename _Period1,typename _Rep2,typename _Period2>
        RandomTimerAdaptor(sharpen::EventEngine &engine,const std::chrono::duration<_Rep1,_Period1> &minWait,const std::chrono::duration<_Rep2,_Period2> &maxWait)
            :RandomTimerAdaptor(engine,minWait,maxWait,std::random_device{}())
        {}

        template<typename _Rep1,typename _Period1,typename _Rep2,typename _Period2>
        RandomTimerAdaptor(sharpen::EventEngine &engine,const std::chrono::duration<_Rep1,_Period1> &minWait,const std::chrono::duration<_Rep2,_Period2> &maxWait,sharpen::Uint32 seed)
            :timer_(sharpen::MakeTimer(engine))
            ,random_(seed)
            ,distrubution_(static_cast<sharpen::Uint32>(minWait/std::chrono::milliseconds(1)),static_cast<sharpen::Uint32>(maxWait/std::chrono::milliseconds(1)))
        {}

        RandomTimerAdaptor(const Self &other)
            :timer_(other.timer_)
            ,random_(other.random_)
            ,distrubution_(other.distrubution_)
        {}

        RandomTimerAdaptor(Self &&other) noexcept
            :timer_(std::move(other.timer_))
            ,random_(std::move(other.random_))
            ,distrubution_(std::move(other.distrubution_))
        {}

        Self &operator=(const Self &other)
        {
            Self tmp{other};
            std::swap(*this,tmp);
            return *this;
        }

        Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->timer_ = std::move(other.timer_);
                this->random_ = std::move(other.random_);
                this->distrubution_ = std::move(other.distrubution_);
            }
            return *this;
        }

        void WaitAsync(sharpen::Future<bool> &future);

        inline bool Await()
        {
            sharpen::AwaitableFuture<bool> future;
            this->WaitAsync(future);
            return future.Await();
        }

        void Cancel();

        ~RandomTimerAdaptor() noexcept = default;
    };
}

#endif