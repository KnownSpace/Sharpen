#pragma once
#ifndef _SHARPEN_ASYNCLEASEEBARRIER_HPP
#define _SHARPEN_ASYNCLEASEBARRIER_HPP

#include "IAsyncBarrier.hpp"
#include "ITimer.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include <cassert>
#include <chrono>
#include <utility>
#include <vector>

namespace sharpen {
    class AsyncLeaseBarrier
        : public sharpen::IAsyncBarrier
        , public sharpen::Noncopyable
        , public sharpen::Nonmovable {
    private:
        using Self = sharpen::AsyncLeaseBarrier;
        using MyFuture = sharpen::AwaitableFuture<std::size_t>;
        using MyFuturePtr = MyFuture *;
        using Waiters = std::vector<MyFuturePtr>;

        std::chrono::milliseconds timeout_;
        std::size_t count_;
        std::size_t currentCount_;
        bool timerStarted_;
        sharpen::TimerPtr timer_;
        sharpen::Future<bool> timeoutFuture_;
        Waiters waiters_;
        sharpen::SpinLock lock_;
        sharpen::BarrierModel model_;

        void TimeoutNotice(sharpen::Future<bool> &future) noexcept;

        void StartTimer();

        void StopTimer();

        void ResetWithoutLock() noexcept;

    public:
        template<typename _Rep, typename _Period>
        AsyncLeaseBarrier(sharpen::TimerPtr timer,
                          const std::chrono::duration<_Rep, _Period> &timeout,
                          std::size_t count)
            : AsyncLeaseBarrier(sharpen::BarrierModel::Flush, std::move(timer), timeout, count) {
        }

        template<typename _Rep, typename _Period>
        AsyncLeaseBarrier(sharpen::BarrierModel model,
                          sharpen::TimerPtr timer,
                          const std::chrono::duration<_Rep, _Period> &timeout,
                          std::size_t count)
            : timeout_(timeout)
            , count_(count)
            , currentCount_(0)
            , timerStarted_(false)
            , timer_(std::move(timer))
            , timeoutFuture_()
            , waiters_()
            , lock_()
            , model_(model) {
            assert(this->count_);
        }

        virtual ~AsyncLeaseBarrier() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        virtual std::size_t WaitAsync() override;

        virtual void Notify(std::size_t count) noexcept override;

        virtual void Reset() noexcept override;

        inline virtual sharpen::BarrierModel GetModel() const noexcept override {
            return this->model_;
        }
    };
}   // namespace sharpen

#endif