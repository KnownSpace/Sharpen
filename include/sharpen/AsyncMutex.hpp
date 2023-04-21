#pragma once
#ifndef _SHARPEN_ASYNCMUTEX_HPP
#define _SHARPEN_ASYNCMUTEX_HPP

#include "AwaitableFuture.hpp"
#include "IAsyncLockable.hpp"
#include <vector>

namespace sharpen
{
    class AsyncMutex
        : public sharpen::Noncopyable
        , public sharpen::Nonmovable
        , public sharpen::IAsyncLockable
    {

    private:
        using MyFuture = sharpen::AwaitableFuture<void>;
        using MyFuturePtr = MyFuture *;
        using Waiters = std::vector<MyFuturePtr>;

        bool locked_;
        Waiters waiters_;
        sharpen::SpinLock lock_;

    public:
        AsyncMutex();

        virtual void LockAsync() override;

        virtual void Unlock() noexcept override;

        bool TryLock() noexcept;

        inline bool try_lock() noexcept
        {
            return this->TryLock();
        }

        virtual ~AsyncMutex() noexcept = default;
    };

}   // namespace sharpen

#endif
