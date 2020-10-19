#pragma once
#ifndef _SHARPEN_ASYNCMUTEX_HPP
#define _SHARPEN_ASYNCMUTEX_HPP

#include <list>

#include "AwaitableFuture.hpp"

namespace sharpen
{
    class AsyncMutex:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
        
    private:
        using MyFuture = sharpen::AwaitableFuture<void>;
        using List = std::list<MyFuture>;

        bool locked_;
        List waiters_;
        sharpen::SpinLock lock_;
    public:
        AsyncMutex();

        void LockAsync();

        void Unlock() noexcept;

        ~AsyncMutex() noexcept = default;
    };
    
}

#endif
