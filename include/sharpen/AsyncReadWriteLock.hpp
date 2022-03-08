#pragma once
#ifndef _SHARPEN_ASYNCREADWRITELOCK_HPP
#define _SHARPEN_ASYNCREADWRITELOCK_HPP

#include <vector>
#include <mutex>

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
        using Self = sharpen::AsyncReadWriteLock;
        using MyFuture = sharpen::AwaitableFuture<sharpen::ReadWriteLockState>;
        using MyFuturePtr = MyFuture*;
        using Waiters = std::vector<MyFuturePtr>;

        sharpen::ReadWriteLockState state_;
        Waiters readWaiters_;
        Waiters writeWaiters_;
        sharpen::SpinLock lock_;
        sharpen::Uint32 readers_;

        void WriteUnlock() noexcept;

        void ReadUnlock() noexcept;

        friend class std::unique_lock<Self>;

        //basic lockable requirement
        //never use me
        //you should use LockWrite or LockRead
        inline void lock()
        {
            this->LockWrite();
        }
    public:
        AsyncReadWriteLock();

        //return prev status
        sharpen::ReadWriteLockState LockRead();

        bool TryLockRead();

        bool TryLockRead(sharpen::ReadWriteLockState &prevStatus);

        //return prev status
        sharpen::ReadWriteLockState LockWrite();

        bool TryLockWrite();

        bool TryLockWrite(sharpen::ReadWriteLockState &prevStatus);

        //upgrade to write lock
        sharpen::ReadWriteLockState UpgradeFromRead();

        void Unlock() noexcept;

        inline void unlock() noexcept
        {
            this->Unlock();
        }

        ~AsyncReadWriteLock() noexcept = default;
    };
    
}

#endif
