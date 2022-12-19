#pragma once
#ifndef _SHARPEN_TIMERREF_HPP
#define _SHARPEN_TIMERREF_HPP

#include <atomic>

#include "ITimerPool.hpp"

namespace sharpen
{
    class UniquedTimerRef:public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::UniquedTimerRef;
        using WaitFuture = sharpen::Future<bool>;
    
        sharpen::ITimerPool *pool_;
        sharpen::TimerPtr timer_;
    public:
    
        UniquedTimerRef(sharpen::ITimerPool &pool);

        UniquedTimerRef(sharpen::TimerPtr &&timer,sharpen::ITimerPool &pool) noexcept;

        UniquedTimerRef(Self &&other) noexcept;

        Self &operator=(Self &&other) noexcept;
    
        ~UniquedTimerRef() noexcept;
    
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
            assert(this->timer_);
            return this->timer_->Await(time);
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

    class SharedTimerRef
    {
    private:
        using Self = sharpen::SharedTimerRef;
        using WaitFuture = sharpen::Future<bool>;

        std::shared_ptr<sharpen::UniquedTimerRef> realTimer_;
    public:
    
        SharedTimerRef(sharpen::ITimerPool &pool);

        SharedTimerRef(sharpen::TimerPtr &&timer,sharpen::ITimerPool &pool) noexcept;
    
        SharedTimerRef(const Self &other) = default;
    
        SharedTimerRef(Self &&other) noexcept = default;
    
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
    
        ~SharedTimerRef() noexcept = default;
    
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
            return this->realTimer_->Await(time);
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