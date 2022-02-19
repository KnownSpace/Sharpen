#pragma once
#ifndef _SHARPEN_ASYNCSEMAPHORE_HPP
#define _SHARPEN_ASYNCSEMAPHORE_HPP

#include <vector>

#include "AwaitableFuture.hpp"
#include "TypeDef.hpp"
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
        sharpen::Uint32 counter_;

        bool NeedWait() const;
    public:
        explicit AsyncSemaphore(sharpen::Uint32 count);

        virtual void LockAsync() override;

        virtual void Unlock() noexcept override;
        
        void Unlock(sharpen::Uint32 count) noexcept;

        ~AsyncSemaphore() noexcept = default;
    };
    
}

#endif
