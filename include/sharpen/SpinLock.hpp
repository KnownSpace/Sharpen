#pragma once
#ifndef _SHARPEN_SPINLOCK_HPP
#define _SHARPEN_SPINLOCK_HPP

#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include <atomic>

namespace sharpen {

    class SpinLock
        : public sharpen::Noncopyable
        , public sharpen::Nonmovable {
    private:
        std::atomic_uint64_t acquireCount_;
        std::atomic_uint64_t releaseCount_;

    public:
        SpinLock() noexcept;

        // use by stl
        void lock() noexcept;

        inline void Lock() noexcept {
            this->lock();
        }

        // use by stl
        void unlock() noexcept;

        inline void Unlock() noexcept {
            this->unlock();
        }

        bool TryLock() noexcept;

        inline bool try_lock() noexcept {
            return this->TryLock();
        }

        ~SpinLock() noexcept = default;
    };

}   // namespace sharpen

#endif