#pragma once
#ifndef _SHARPEN_LINUXTIMER_HPP
#define _SHARPEN_LINUXTIMER_HPP

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_LINUX

#include <atomic>

#include "ITimer.hpp"
#include "IChannel.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"

#define SHARPEN_HAS_TIMERFD

namespace sharpen
{
    //use timer fd
    class LinuxTimer:public sharpen::ITimer,public sharpen::IChannel,public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Mybase = sharpen::IChannel;
        using MyTimerBase = sharpen::ITimer;

        sharpen::Future<bool> *future_;
    public:
        LinuxTimer();

        virtual ~LinuxTimer() noexcept = default;

        virtual void OnEvent(sharpen::IoEvent *event) override;

        virtual void WaitAsync(sharpen::Future<bool> &future,sharpen::Uint64 waitMs);

        virtual void Cancel() override;
    };
}

#endif
#endif