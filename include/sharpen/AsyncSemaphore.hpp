#pragma once
#ifndef _SHARPEN_ASYNCSEMAPHORE_HPP
#define _SHARPEN_ASYNCSEMAPHORE_HPP

#include "AwaitableFuture.hpp"
#include "IAsyncLockable.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace sharpen
{

    class AsyncSemaphore
        : public sharpen::Noncopyable
        , public sharpen::Nonmovable
        , public sharpen::IAsyncLockable
    {
    private:
        using MyFuture = sharpen::AwaitableFuture<void>;
        using MyFuturePtr = MyFuture *;
        using Waiters = std::vector<MyFuturePtr>;

        Waiters waiters_;
        sharpen::SpinLock lock_;
        std::size_t counter_;

        bool NeedWait() const;

    public:
        explicit AsyncSemaphore(std::size_t count);

        virtual void LockAsync() override;

        virtual void Unlock() noexcept override;

        void Unlock(std::size_t count) noexcept;

        bool TryLock() noexcept;

        inline bool try_lock() noexcept
        {
            return this->TryLock();
        }

        virtual ~AsyncSemaphore() noexcept = default;
    };

}   // namespace sharpen

#endif
