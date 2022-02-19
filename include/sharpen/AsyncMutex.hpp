#pragma once
#ifndef _SHARPEN_ASYNCMUTEX_HPP
#define _SHARPEN_ASYNCMUTEX_HPP

#include <vector>

#include "AwaitableFuture.hpp"
#include "IAsyncLockable.hpp"

namespace sharpen
{
    class AsyncMutex:public sharpen::Noncopyable,public sharpen::Nonmovable,public sharpen::IAsyncLockable
    {
        
    private:
        using MyFuture = sharpen::AwaitableFuture<void>;
        using MyFuturePtr = MyFuture*;
        using Waiters = std::vector<MyFuturePtr>;

        bool locked_;
        Waiters waiters_;
        sharpen::SpinLock lock_;
    public:
        AsyncMutex();

        virtual void LockAsync() override;

        virtual void Unlock() noexcept override;

        ~AsyncMutex() noexcept = default;
    };
    
}

#endif
