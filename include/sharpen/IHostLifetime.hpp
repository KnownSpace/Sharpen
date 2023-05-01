#pragma once
#ifndef _SHARPEN_IHOSTLIFETIME_HPP
#define _SHARPEN_IHOSTLIFETIME_HPP

#include "IHost.hpp"

namespace sharpen {
    class IHostLifetime {
    private:
        using Self = sharpen::IHostLifetime;

    protected:
    public:
        IHostLifetime() noexcept = default;

        IHostLifetime(const Self &other) noexcept = default;

        IHostLifetime(Self &&other) noexcept = default;

        Self &operator=(const Self &other) noexcept = default;

        Self &operator=(Self &&other) noexcept = default;

        virtual ~IHostLifetime() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        virtual void Bind(sharpen::IHost &host) = 0;

        virtual void Run() = 0;
    };
}   // namespace sharpen

#endif