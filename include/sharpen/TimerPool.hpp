#pragma once
#ifndef _SHARPEN_TIMERPOOL_HPP
#define _SHARPEN_TIMERPOOL_HPP

#include <vector>
#include <mutex>

#include "ITimer.hpp"

namespace sharpen
{
    class TimerPool:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Self = TimerPool;
        using TimerMaker = sharpen::TimerPtr(*)(sharpen::EventEngine*);
    
        sharpen::SpinLock lock_;
        std::vector<sharpen::TimerPtr> timers_;
        TimerMaker maker_;
        sharpen::EventEngine *engine_;

        inline sharpen::TimerPtr MakeTimer() const
        {
            if(this->maker_)
            {
                return this->maker_(this->engine_);
            }
            return sharpen::MakeTimer(*this->engine_);
        }
    public:

        explicit TimerPool(sharpen::EventEngine &engine)
            :TimerPool(engine,static_cast<sharpen::Size>(0))
        {}

        TimerPool(sharpen::EventEngine &engine,sharpen::Size reserveCount)
            :TimerPool(engine,nullptr,reserveCount)
        {}

        TimerPool(sharpen::EventEngine &engine,TimerMaker maker)
            :TimerPool(engine,maker,0)
        {}

        TimerPool(sharpen::EventEngine &engine,TimerMaker maker,sharpen::Size reserveCount);
    
        ~TimerPool() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        sharpen::TimerPtr GetTimer();

        void Reserve(sharpen::Size size);

        void PutTimer(sharpen::TimerPtr &&timer);
    };   
}

#endif