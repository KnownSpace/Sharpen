#pragma once
#ifndef _SHARPEN_ASYNCBARRIER_HPP
#define _SHARPEN_ASYNCBARRIER_HPP

#include "AwaitableFuture.hpp"
#include "IAsyncBarrier.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace sharpen
{
    class AsyncBarrier
        : public sharpen::IAsyncBarrier
        , public sharpen::Noncopyable
        , public sharpen::Nonmovable
    {
    private:
        using MyFuture = sharpen::AwaitableFuture<std::size_t>;
        using MyFuturePtr = MyFuture *;
        using Waiters = std::vector<MyFuturePtr>;

        std::size_t count_;
        Waiters waiters_;
        std::size_t currentCount_;
        sharpen::SpinLock lock_;
        sharpen::BarrierModel model_;

        void ResetWithoutLock() noexcept;

    public:
        AsyncBarrier(std::size_t count)
            : AsyncBarrier(sharpen::BarrierModel::Flush, count)
        {
        }

        AsyncBarrier(sharpen::BarrierModel model, std::size_t count);

        virtual std::size_t WaitAsync() override;

        virtual void Notify(std::size_t count) noexcept override;

        virtual void Reset() noexcept override;

        virtual ~AsyncBarrier() noexcept = default;

        inline virtual sharpen::BarrierModel GetModel() const noexcept override
        {
            return this->model_;
        }
    };

}   // namespace sharpen

#endif
