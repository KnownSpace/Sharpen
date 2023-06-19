#pragma once
#ifndef _SHARPEN_ASYNCLEASELOCK_HPP
#define _SHARPEN_ASYNCLEASELOCK_HPP

#include "AsyncMutex.hpp"
#include "IAsyncLockable.hpp"
#include "ITimer.hpp"
#include "SpinLock.hpp"
#include "IntOps.hpp"
#include <chrono>
#include <stdexcept>   // IWYU pragma: export
#include <utility>


namespace sharpen {
    // this lock should not be used in thread sync
    class AsyncLeaseLock
        : public sharpen::IAsyncLockable
        , public sharpen::Noncopyable
        , public sharpen::Nonmovable {
    private:
        using Self = sharpen::AsyncLeaseLock;
        using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

        std::chrono::milliseconds leaseDuration_;
        sharpen::SpinLock lock_;
        sharpen::AsyncMutex queueLock_;
        std::uint64_t ownerId_;
        TimePoint grantTime_;
        sharpen::ITimer *waiter_;

        explicit AsyncLeaseLock(std::uint32_t waitMs);

    public:
        template<typename _Rep, typename _Period>
        explicit AsyncLeaseLock(const std::chrono::duration<_Rep, _Period> &leaseDuration)
            : Self{sharpen::IntCast<std::uint32_t>(leaseDuration.count())} {
        }

        virtual ~AsyncLeaseLock() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        void LockAsync(sharpen::TimerPtr timer);

        virtual void LockAsync() override;

        virtual void Unlock() noexcept override;
    };
}   // namespace sharpen

#endif