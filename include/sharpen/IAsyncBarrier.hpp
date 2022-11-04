#pragma once
#ifndef _SHARPEN_IASYNCBARRIER_HPP
#define _SHARPEN_IASYNCBARRIER_HPP

#include <cstddef>

namespace sharpen
{
    class IAsyncBarrier
    {
    private:
        using Self = sharpen::IAsyncBarrier;
    protected:
    public:
    
        IAsyncBarrier() noexcept = default;
    
        IAsyncBarrier(const Self &other) noexcept = default;
    
        IAsyncBarrier(Self &&other) noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    
        virtual ~IAsyncBarrier() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        virtual void Notify(std::size_t count) noexcept = 0;

        inline void NotifyOnce() noexcept
        {
            this->Notify(1);
        }

        virtual std::size_t WaitAsync() = 0;

        virtual void Reset() noexcept = 0;
    };    
}

#endif