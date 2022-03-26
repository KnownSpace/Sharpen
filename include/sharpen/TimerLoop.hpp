#pragma once
#ifndef _SHARPEN_TIMERLOOP_HPP
#define _SHARPEN_TIMERLOOP_HPP

#include "ITimer.hpp"

namespace sharpen
{
    class TimerLoop:public sharpen::Noncopyable
    {
    public:
        
        enum class LoopStatus
        {
            Continue,
            Terminate
        };
    private:
        using Self = sharpen::TimerLoop;
        using WaitTime = std::chrono::milliseconds;
        using Handler = std::function<LoopStatus()>;
    
        sharpen::TimerPtr timer_;
        Handler handler_;
        std::function<WaitTime()> waitTimeGenerator_;
        sharpen::AwaitableFuturePtr<void> future_;
        std::atomic_bool token_;
        sharpen::EventEngine *engine_;

        class DefaultGenerator
        {
        private:
            using Self = DefaultGenerator;
        
            WaitTime time_;
        public:
            explicit DefaultGenerator(WaitTime time)
                :time_(std::move(time))
            {}
        
            DefaultGenerator(const Self &other) = default;
        
            DefaultGenerator(Self &&other) noexcept = default;
        
            inline Self &operator=(const Self &other)
            {
                Self tmp{other};
                std::swap(tmp,*this);
                return *this;
            }
        
            inline Self &operator=(Self &&other) noexcept
            {
                if(this != std::addressof(other))
                {
                    this->time_ = std::move(other.time_);
                }
                return *this;
            }
        
            ~DefaultGenerator() noexcept = default;

            inline WaitTime operator()() const noexcept
            {
                return this->time_;
            }
        };

        void Entry();
    public:

        template<typename _Rep,typename _Period>
        TimerLoop(sharpen::EventEngine &engine,sharpen::TimerPtr timer,const std::chrono::duration<_Rep,_Period> &duration,Handler handler)
            :TimerLoop(engine,std::move(timer),std::move(handler),Self::DefaultGenerator{duration})
        {}

        TimerLoop(sharpen::EventEngine &engine,sharpen::TimerPtr timer,Handler handler,std::function<WaitTime()> waittimeGenerator);
    
        TimerLoop(Self &&other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept;
    
        inline ~TimerLoop() noexcept
        {
            this->Terminate();
        }

        void Cancel();

        void Stop();

        void Terminate();

        void Restart();
    };
}

#endif