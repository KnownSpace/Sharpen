#pragma once
#ifndef _SHARPEN_ASYNCREADWRITELOCK_HPP
#define _SHARPEN_ASYNCREADWRITELOCK_HPP

#include <vector>

#include "AwaitableFuture.hpp"

namespace sharpen
{
    enum class ReadWriteLockState
    {
        Free,
        SharedReading,
        UniquedWriting
    };

    class AsyncReadWriteLock:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using MyFuture = sharpen::AwaitableFuture<void>;
        using MyFuturePtr = MyFuture*;
        using Waiters = std::vector<MyFuturePtr>;

        sharpen::ReadWriteLockState state_;
        Waiters readWaiters_;
        Waiters writeWaiters_;
        sharpen::SpinLock lock_;
        sharpen::Uint32 readers_;

        void WriteUnlock() noexcept;

        void ReadUnlock() noexcept;
    public:
        AsyncReadWriteLock();

        void LockRead();

        bool TryLockRead();

        void LockWrite();

        bool TryLockWrite();

        void Unlock() noexcept;

        inline void unlock() noexcept
        {
            this->Unlock();
        }

        ~AsyncReadWriteLock() noexcept = default;
    };
    
}

#endif
