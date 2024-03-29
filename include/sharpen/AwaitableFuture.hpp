#pragma once
#ifndef _SHARPEN_AWAITABLEFUTURE_HPP
#define _SHARPEN_AWAITABLEFUTURE_HPP

#include "Future.hpp"
#include "IFiberScheduler.hpp"
#include <cassert>

namespace sharpen {
    template<typename _Result>
    class AwaitableFuture : public sharpen::Future<_Result> {
    private:
        using MyBase = sharpen::Future<_Result>;
        using Self = sharpen::AwaitableFuture<_Result>;

        sharpen::FiberPtr pendingFiber_;
        sharpen::FiberPtr awaiter_;

        static void ScheduleFiber(sharpen::FiberPtr &&fiber) {
            sharpen::IFiberScheduler *scheduler = fiber->GetScheduler();
            assert(scheduler);
            scheduler->Schedule(std::move(fiber));
        }

        void NotifyIfCompleted() noexcept {
            sharpen::FiberPtr fiber{nullptr};
            {
                std::unique_lock<sharpen::SpinLock> lock(this->GetCompleteLock());
                if (this->IsPending()) {
                    this->awaiter_ = std::move(this->pendingFiber_);
                    return;
                }
                fiber = std::move(this->pendingFiber_);
            }
            this->ScheduleFiber(std::move(fiber));
        }

    public:
        AwaitableFuture()
            : MyBase()
            , pendingFiber_(nullptr)
            , awaiter_(nullptr) {
        }

        AwaitableFuture(Self &&other) noexcept = default;

        inline Self &operator=(Self &&other) noexcept {
            if (this != std::addressof(other)) {
                MyBase::operator=(std::move(other));
                this->pendingFiber_ = std::move(other.pendingFiber_);
                this->awaiter_ = std::move(other.awaiter_);
            }
            return *this;
        }

        virtual ~AwaitableFuture() noexcept = default;

        inline void WaitAsync() {
            sharpen::FiberPtr current = sharpen::Fiber::GetCurrentFiber();
            sharpen::IFiberScheduler *scheduler = current->GetScheduler();
            // this thread is not a processer
            if (!scheduler || !scheduler->IsProcesser()) {
                this->Wait();
            } else {
                // this thread is a processer
                bool pending{false};
                {
                    std::unique_lock<sharpen::SpinLock> lock{this->GetCompleteLock()};
                    pending = this->IsPending();
                    if (pending) {
                        this->pendingFiber_ = std::move(current);
                    }
                }
                if (pending) {
                    scheduler->SetSwitchCallback(
                        std::bind(&sharpen::AwaitableFuture<_Result>::NotifyIfCompleted, this));
                    scheduler->SwitchToProcesserFiber();
                }
            }
        }

        inline auto Await() -> decltype(this->Get()) {
            this->WaitAsync();
            return this->Get();
        }

    protected:
        inline virtual void ExecuteCallback(sharpen::FutureState state) override {
            typename MyBase::Callback cb;
            sharpen::FiberPtr fiber{nullptr};
            {
                std::unique_lock<sharpen::SpinLock> lock{this->GetCompleteLock()};
                std::swap(cb, this->GetCallback());
                std::swap(fiber, this->awaiter_);
                this->SetState(state);
            }
            if (cb) {
                cb(*this);
            }
            if (fiber) {
                Self::ScheduleFiber(std::move(fiber));
            }
        }
    };

    template<typename _T>
    using AwaitableFuturePtr = std::shared_ptr<sharpen::AwaitableFuture<_T>>;

    template<typename _T>
    inline sharpen::AwaitableFuturePtr<_T> MakeAwaitableFuture() {
        return std::make_shared<sharpen::AwaitableFuture<_T>>();
    }
}   // namespace sharpen

#endif