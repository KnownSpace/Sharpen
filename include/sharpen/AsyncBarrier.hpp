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

        sharpen::Uint64 counter_;
        List waiters_;
        sharpen::Uint64 beginCounter_;
        sharpen::SpinLock lock_;
    public:
        explicit AsyncBarrier(sharpen::Uint64 count);

        void WaitAsync();
        
        void Notice() noexcept;
        
        void Reset();

        ~AsyncBarrier() noexcept = default;
    };
    
}

#endif
