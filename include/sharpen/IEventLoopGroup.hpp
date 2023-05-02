#pragma once
#ifndef _SHARPEN_IEVENTLOOPGROUP_HPP
#define _SHARPEN_IEVENTLOOPGROUP_HPP

#include "EventLoop.hpp"

namespace sharpen {
    class IEventLoopGroup {
    private:
        using Self = sharpen::IEventLoopGroup;

    protected:
    public:
        IEventLoopGroup() noexcept = default;

        IEventLoopGroup(const Self &other) noexcept = default;

        IEventLoopGroup(Self &&other) noexcept = default;

        Self &operator=(const Self &other) noexcept = default;

        Self &operator=(Self &&other) noexcept = default;

        virtual ~IEventLoopGroup() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        virtual sharpen::EventLoop &RoundRobinLoop() noexcept = 0;

        virtual std::size_t GetLoopCount() const noexcept = 0;

        virtual void Run() = 0;

        virtual void Stop() noexcept = 0;
    };
}   // namespace sharpen

#endif