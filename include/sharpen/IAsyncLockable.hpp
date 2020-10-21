#pragma once
#ifndef _SHARPEN_IASYNCLOCKABLE_HPP
#define _SHARPEN_IASYNCLOCKABLE_HPP

namespace sharpen
{
    class IAsyncLockable
    {
    private:
        using Self = sharpen::IAsyncLockable;
    public:
        IAsyncLockable() noexcept = default;
        
        IAsyncLockable(Self &&) noexcept = default;
        
        IAsyncLockable(const Self &) = default;
        
        virtual ~IAsyncLockable() = default;
        
        virtual void LockAsync() = 0;
        
        virtual void Unlock() noexcept = 0;
        
        //used by stl
        inline void lock()
        {
            this->LockAsync();
        }
        
        //used by stl
        inline void unlock() noexcept
        {
            this->Unlock();
        }
    };
}

#endif
