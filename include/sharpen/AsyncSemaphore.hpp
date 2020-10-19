#pragma once
#ifndef _SHARPEN_ASYNCSEMAPHORE_HPP
#define _SHARPEN_ASYNCSEMAPHORE_HPP

#include <list>

#include "AwaitableFuture.hpp"
#include "TypeDef.hpp"

namespace sharpen
{

    class AsyncSemaphore:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using MyFuture = sharpen::AwaitableFuture<void>;
        using MyFuturePtr = MyFuture*;
        using List = std::list<MyFuturePtr>;

        List waiters_;
        sharpen::SpinLock lock_;
        sharpen::Uint32 counter_;

        bool NeedWait() const;
    public:
        AsyncSemaphore(sharpen::Uint32 count);

        void LockAsync();

        void Unlock() noexcept;

        ~AsyncSemaphore() noexcept = default;
    };
    
}

#endif
