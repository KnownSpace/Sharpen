#pragma once
#ifndef _SHARPEN_ASYNCSEMAPHORE_HPP
#define _SHARPEN_ASYNCSEMAPHORE_HPP

#include <vector>

#include "AwaitableFuture.hpp"
#include <cstdint>
#include <cstddef>
#include "IAsyncLockable.hpp"

namespace sharpen
{

    class AsyncSemaphore:public sharpen::Noncopyable,public sharpen::Nonmovable,public sharpen::IAsyncLockable
    {
    private:
        using MyFuture = sharpen::AwaitableFuture<void>;
        using MyFuturePtr = MyFuture*;
        using Waiters = std::vector<MyFuturePtr>;

        Waiters waiters_;
        sharpen::SpinLock lock_;
        std::uint32_t counter_;

        bool NeedWait() const;
    public:
        explicit AsyncSemaphore(std::uint32_t count);

        virtual void LockAsync() override;

        virtual void Unlock() noexcept override;
        
        void Unlock(std::uint32_t count) noexcept;

        bool TryLock();

        inline bool try_lock()
        {
            return this->TryLock();
        }

        ~AsyncSemaphore() noexcept = default;
    };
    
}

#endif
