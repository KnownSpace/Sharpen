#pragma once
#ifndef _SHARPEN_LEASELOCK_HPP
#define _SHARPEN_LEASELOCK_HPP

#include "SpinLock.hpp"
#include "IAsyncLockable.hpp"
#include "AsyncMutex.hpp"
#include "ITimer.hpp"
#include <utility>
#include <chrono>
#include <stdexcept> // IWYU pragma: export

namespace sharpen {
    // this lock should not be used in thread sync
    class LeaseLock:public sharpen::IAsyncLockable {
    private:
        using Self = sharpen::LeaseLock;
        using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

        std::chrono::milliseconds leaseDuration_;
        sharpen::SpinLock lock_;
        sharpen::AsyncMutex queueLock_;
        std::uint64_t ownerId_;
        TimePoint grantTime_;
        sharpen::ITimer *waiter_;

        explicit LeaseLock(std::uint32_t waitMs);
    public:
        template<typename _Rep,typename _Period>
        explicit LeaseLock(const std::chrono::duration<_Rep,_Period> &leaseDuration) 
            : Self{leaseDuration.count()}{
        }

        virtual ~LeaseLock() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        void LockAsync(sharpen::TimerPtr timer);

        virtual void LockAsync() override;

        virtual void Unlock() noexcept override;
    };
}   // namespace sharpen

#endif