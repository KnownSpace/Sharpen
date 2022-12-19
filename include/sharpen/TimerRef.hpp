#pragma once
#ifndef _SHARPEN_TIMERREF_HPP
#define _SHARPEN_TIMERREF_HPP

#include <atomic>

#include "ITimerPool.hpp"

namespace sharpen
{
    class TimerUniquedRef:public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::TimerUniquedRef;
        using WaitFuture = sharpen::Future<bool>;
    
        sharpen::ITimerPool *pool_;
        sharpen::TimerPtr timer_;
    public:
    
        TimerUniquedRef(sharpen::ITimerPool &pool);

        TimerUniquedRef(sharpen::TimerPtr &&timer,sharpen::ITimerPool &pool) noexcept;

        TimerUniquedRef(Self &&other) noexcept;

        Self &operator=(Self &&other) noexcept;
    
        ~TimerUniquedRef() noexcept;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline void WaitAsync(WaitFuture &future,std::uint64_t waitMs)
        {
            assert(this->timer_);
            this->timer_->WaitAsync(future,waitMs);
        }

        inline void Cancel()
        {
            assert(this->timer_);
            this->timer_->Cancel();
        }

        template<typename _Rep,typename _Period>
        inline void WaitAsync(WaitFuture &future,const std::chrono::duration<_Rep,_Period> &time)
        {
            assert(this->timer_);
            this->timer_->WaitAsync(future,time/std::chrono::milliseconds(1));
        }

        template<typename _Rep,typename _Period>
        inline bool Await(const std::chrono::duration<_Rep,_Period> &time)
        {
            sassert(this->timer_);
            this->timer_->Await(time);
        }

        inline sharpen::TimerPtr &Timer() noexcept
        {
            return this->timer_;
        }
        
        inline const sharpen::TimerPtr &Timer() const noexcept
        {
            return this->timer_;
        }
    };

    class TimerRef
    {
    private:
        using Self = sharpen::TimerRef;
        using WaitFuture = sharpen::Future<bool>;

        std::shared_ptr<sharpen::TimerUniquedRef> realTimer_;
    public:
    
        TimerRef(sharpen::ITimerPool &pool);

        TimerRef(sharpen::TimerPtr &&timer,sharpen::ITimerPool &pool) noexcept;
    
        TimerRef(const Self &other) = default;
    
        TimerRef(Self &&other) noexcept = default;
    
        inline Self &operator=(const Self &other)
        {
            if(this != std::addressof(other))
            {
                Self tmp{other};
                std::swap(tmp,*this);
            }
            return *this;
        }
    
        Self &operator=(Self &&other) noexcept;
    
        ~TimerRef() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline void WaitAsync(WaitFuture &future,std::uint64_t waitMs)
        {
            assert(this->realTimer_);
            this->realTimer_->WaitAsync(future,waitMs);
        }

        inline void Cancel()
        {
            assert(this->realTimer_);
            this->realTimer_->Cancel();
        }

        template<typename _Rep,typename _Period>
        inline void WaitAsync(WaitFuture &future,const std::chrono::duration<_Rep,_Period> &time)
        {
            assert(this->realTimer_);
            this->realTimer_->WaitAsync(future,time/std::chrono::milliseconds(1));
        }

        template<typename _Rep,typename _Period>
        inline bool Await(const std::chrono::duration<_Rep,_Period> &time)
        {
            assert(this->realTimer_);
            this->realTimer_->Await(time);
        }

        inline sharpen::TimerPtr &Timer() noexcept
        {
            assert(this->realTimer_);
            return this->realTimer_->Timer();
        }
        
        inline const sharpen::TimerPtr &Timer() const noexcept
        {
            assert(this->realTimer_);
            return this->realTimer_->Timer();
        }
    };
}

#endif