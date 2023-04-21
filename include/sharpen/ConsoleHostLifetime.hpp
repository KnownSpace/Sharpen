#pragma once
#ifndef _SHARPEN_CONSOLEHOSTLIFETIME_HPP
#define _SHARPEN_CONSOLEHOSTLIFETIME_HPP

#include <stdexcept>

#include "IHostLifetime.hpp"
#include "ISignalChannel.hpp"

namespace sharpen
{
    class ConsoleHostLifetime
        : public sharpen::IHostLifetime
        , public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::ConsoleHostLifetime;

        void WaitForSignal(sharpen::SignalChannelPtr channel);

        sharpen::IFiberScheduler *scheduler_;
        sharpen::IEventLoopGroup *loopGroup_;
        sharpen::IHost *host_;

    public:
        ConsoleHostLifetime();

        ConsoleHostLifetime(sharpen::IFiberScheduler &scheduler,
                            sharpen::IEventLoopGroup &loopGroup);

        ConsoleHostLifetime(Self &&other) noexcept;

        Self &operator=(Self &&other) noexcept;

        virtual ~ConsoleHostLifetime() noexcept = default;

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        virtual void Bind(sharpen::IHost &host) override;

        virtual void Run() override;
    };
}   // namespace sharpen

#endif