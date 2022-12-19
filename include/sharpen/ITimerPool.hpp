#pragma once
#ifndef _SHARPEN_ITIMERPOOL_HPP
#define _SHARPEN_ITIMERPOOL_HPP

#include "ITimer.hpp"

namespace sharpen
{
    class ITimerPool
    {
    private:
        using Self = ITimerPool;
    protected:
    public:
    
        ITimerPool() noexcept = default;
    
        ITimerPool(const Self &other) noexcept = default;
    
        ITimerPool(Self &&other) noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    
        virtual ~ITimerPool() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        virtual sharpen::TimerPtr GetTimer() = 0;

        virtual void Reserve(std::size_t size) = 0;

        virtual void ReturnTimer(sharpen::TimerPtr &&timer) noexcept = 0;
    };   
}

#endif