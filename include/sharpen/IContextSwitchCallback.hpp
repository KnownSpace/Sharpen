#pragma once
#ifndef _SHARPEN_ICONTEXTSWITCHCALLBACK_HPP
#define _SHARPEN_ICONTEXTSWITCHCALLBACK_HPP

namespace sharpen
{
    class IContextSwitchCallback
    {
    private:
        using Self = sharpen::IContextSwitchCallback;
    public:
        IContextSwitchCallback() noexcept = default;

        IContextSwitchCallback(const Self &other) noexcept = default;

        IContextSwitchCallback(Self &&other) noexcept = default;

        virtual ~IContextSwitchCallback() noexcept = default;

        Self &operator=(const Self &other) noexcept
        {
            return *this;
        }

        Self &operator=(Self &&other) noexcept
        {
            return *this;
        }

        virtual void Run() noexcept = 0;

        void operator()() noexcept
        {
            this->Run();
        }
    };
}

#endif
