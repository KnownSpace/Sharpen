#pragma once
#ifndef _SHARPEN_ASYNCREADWRITELOCK_HPP
#define _SHARPEN_ASYNCREADWRITELOCK_HPP

#include <functional>
#include <list>

#include "Future.hpp"

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
        using Function = std::function<void()>;
        using List = std::list<Function>;

        sharpen::ReadWriteLockState state_;
        List readWaiters_;
        List writeWaiters_;
        sharpen::SpinLock lock_;
        sharpen::Uint32 readers_;

        void WriteUnlock();

        void ReadUnlock();

        void NoticeAllReaders(List &newList);

        void NoticeWriter(Function &callback);
    public:
        AsyncReadWriteLock();

        void ReadLock(Function &&callback);

        void WriteLock(Function &&callback);

        sharpen::SharedFuturePtr<void> ReadLockAsync();

        sharpen::SharedFuturePtr<void> WriteLockAsync();

        void Unlock();

        ~AsyncReadWriteLock() = default;
    };
    
}

#endif