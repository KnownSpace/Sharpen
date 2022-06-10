#pragma once
#ifndef _SHARPEN_ASYNCBARRIER_HPP
#define _SHARPEN_ASYNCBARRIER_HPP

#include <vector>

#include <cstdint>
#include <cstddef>
#include "AwaitableFuture.hpp"

namespace sharpen
{
    class AsyncBarrier:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using MyFuture = sharpen::AwaitableFuture<void>;
        using MyFuturePtr = MyFuture*;
        using Waiters = std::vector<MyFuturePtr>;

        std::uint64_t counter_;
        Waiters waiters_;
        std::uint64_t beginCounter_;
        sharpen::SpinLock lock_;
    public:
        explicit AsyncBarrier(std::uint64_t count);

        void WaitAsync();
        
        void Notice() noexcept;
        
        void Reset();

        void AddCount(std::uint64_t count);

        ~AsyncBarrier() noexcept = default;
    };
    
}

#endif
