#pragma once
#ifndef _SHARPEN_ASYNCBARRIER_HPP
#define _SHARPEN_ASYNCBARRIER_HPP

#include <vector>
#include <cstdint>
#include <cstddef>

#include "AwaitableFuture.hpp"
#include "IAsyncBarrier.hpp"

namespace sharpen
{
    class AsyncBarrier:public sharpen::IAsyncBarrier,public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using MyFuture = sharpen::AwaitableFuture<std::size_t>;
        using MyFuturePtr = MyFuture*;
        using Waiters = std::vector<MyFuturePtr>;

        std::size_t count_;
        Waiters waiters_;
        std::size_t currentCount_;
        sharpen::SpinLock lock_;

        void ResetWithoutLock() noexcept;
    public:
        explicit AsyncBarrier(std::size_t count);

        virtual std::size_t WaitAsync() override;
        
        virtual void Notify(std::size_t count) noexcept override;
        
        virtual void Reset() noexcept override;

        ~AsyncBarrier() noexcept = default;
    };
    
}

#endif
