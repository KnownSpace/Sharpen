#pragma once
#ifndef _SHARPEN_ASYNCREADWRITELOCK_HPP
#define _SHARPEN_ASYNCREADWRITELOCK_HPP

#include <list>

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
        using List = std::list<MyFuturePtr>;

        sharpen::ReadWriteLockState state_;
        List readWaiters_;
        List writeWaiters_;
        sharpen::SpinLock lock_;
        sharpen::Uint32 readers_;

        void WriteUnlock() noexcept;

        void ReadUnlock() noexcept;
    public:
        AsyncReadWriteLock();

        void LockReadAsync();

        void LockWriteAsync();

        void Unlock() noexcept;

        ~AsyncReadWriteLock() noexcept = default;
    };
    
}

#endif
