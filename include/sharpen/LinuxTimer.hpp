#pragma once
#ifndef _SHARPEN_LINUXTIMER_HPP
#define _SHARPEN_LINUXTIMER_HPP

#include "SystemMacro.hpp"   // IWYU pragma: keep

#ifdef SHARPEN_IS_LINUX

#include "IChannel.hpp"
#include "ITimer.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include <atomic>

#define SHARPEN_HAS_TIMERFD

namespace sharpen {
    // use timer fd
    class LinuxTimer
        : public sharpen::ITimer
        , public sharpen::IChannel
        , public sharpen::Noncopyable
        , public sharpen::Nonmovable {
    private:
        using Mybase = sharpen::IChannel;
        using MyTimerBase = sharpen::ITimer;

        std::atomic<sharpen::Future<bool> *> future_;

    public:
        LinuxTimer();

        virtual ~LinuxTimer() noexcept = default;

        virtual void OnEvent(sharpen::IoEvent *event) override;

        virtual void WaitAsync(sharpen::Future<bool> &future, std::uint64_t waitMs);

        virtual void Cancel() override;
    };
}   // namespace sharpen

#endif
#endif