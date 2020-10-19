#pragma once
#ifndef _SHARPEN_ASYNCBARRIER_HPP
#define _SHARPEN_ASYNCBARRIER_HPP

#include <list>

#include "TypeDef.hpp"
#include "AwaitableFuture.hpp"

namespace sharpen
{
    class AsyncBarrier:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using MyFuture = sharpen::AwaitableFuture<void>;
        using MyFuturePtr = MyFuture*;
        using List = std::list<MyFuturePtr>;

        sharpen::Uint32 counter_;
        List waiters_;
        sharpen::Uint32 beginCounter_;
        sharpen::SpinLock lock_;
    public:
        AsyncBarrier(sharpen::Uint32 count);

        void WaitAsync();
        
        void Notice() noexcept;
        
        void Reset();

        ~AsyncBarrier() noexcept = default;
    };
    
}

#endif
