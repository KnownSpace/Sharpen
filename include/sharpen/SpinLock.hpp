#pragma once
#ifndef _SHARPEN_SPINLOCK_HPP
#define _SHARPEN_SPINLOCK_HPP

#include <atomic>

#include "Noncopyable.hpp"
#include "Nonmovable.hpp"

namespace sharpen
{

    class SpinLock:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Flag = std::atomic_flag;

        Flag flag_;
    public:
        SpinLock() noexcept = default;

        //use by stl
        void lock() noexcept;

        inline void Lock() noexcept
        {
            this->lock();
        }

        //use by stl
        void unlock() noexcept;

        inline void Unlock() noexcept
        {
            this->unlock();
        }

        bool TryLock();

        inline bool try_lock()
        {
            return this->TryLock();
        }

        ~SpinLock() noexcept = default;
    };

} 

#endif