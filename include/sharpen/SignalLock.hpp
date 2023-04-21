#pragma once
#ifndef _SHARPEN_SIGNALLOCK_HPP
#define _SHARPEN_SIGNALLOCK_HPP

#include "IntOps.hpp" // IWYU pragma: keep
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "SystemMacro.hpp"
#include <chrono>
#include <mutex>

#ifndef SHARPEN_IS_WIN
#include <signal.h>
#endif

namespace sharpen
{
    class SignalLock
        : public sharpen::Noncopyable
        , public sharpen::Nonmovable
    {
    private:
        using Self = sharpen::SignalLock;

        std::mutex lock_;
#ifndef SHARPEN_IS_WIN
        static thread_local sigset_t oldSet_;

        static void GetBlockableSet(sigset_t &set) noexcept;
#endif
    public:
        SignalLock() = default;

        ~SignalLock() noexcept = default;

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        void Lock() noexcept;

        inline void lock() noexcept
        {
            this->Lock();
        }

        void Unlock() noexcept;

        inline void unlock() noexcept
        {
            this->Unlock();
        }
    };
}   // namespace sharpen

#endif