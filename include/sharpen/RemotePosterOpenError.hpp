#pragma once
#ifndef _SHARPEN_REMOTEPOSTEROPENEDERROR_HPP
#define _SHARPEN_REMOTEPOSTEROPENEDERROR_HPP

#include <exception>

namespace sharpen
{
    template<typename _Exception, bool _IsEmpty>
    class InternalRemotePosterOpenError : public _Exception
    {
    private:
        using Self = InternalRemotePosterOpenError<_Exception, _IsEmpty>;

        const char *msg_;

    public:
        InternalRemotePosterOpenError() noexcept = default;

        explicit InternalRemotePosterOpenError(const char *msg)
            : msg_(msg)
        {
        }

        InternalRemotePosterOpenError(const Self &other) noexcept = default;

        InternalRemotePosterOpenError(Self &&other) noexcept = default;

        ~InternalRemotePosterOpenError() noexcept = default;

        Self &operator=(const Self &other) noexcept = default;

        Self &operator=(Self &&other) noexcept = default;

        virtual const char *what() const noexcept override
        {
            return this->msg_;
        }
    };

    // msvc exception
    template<typename _Exception>
    class InternalRemotePosterOpenError<_Exception, false> : public _Exception
    {
    private:
        using Self = InternalRemotePosterOpenError<_Exception, false>;
        using Base = _Exception;

    public:
        InternalRemotePosterOpenError() noexcept = default;

        explicit InternalRemotePosterOpenError(const char *msg) noexcept
            : Base(msg)
        {
        }

        InternalRemotePosterOpenError(const Self &other) noexcept = default;

        InternalRemotePosterOpenError(Self &&other) noexcept = default;

        ~InternalRemotePosterOpenError() noexcept = default;

        Self &operator=(const Self &other) noexcept = default;

        Self &operator=(Self &&other) noexcept = default;
    };

    // if sizeof(std::exception) == sizeof(void*),std::exception is a interface class
    // exception class defination
    class RemotePosterOpenError
        : public InternalRemotePosterOpenError<std::exception,
                                               sizeof(std::exception) == sizeof(void *)>
    {
    private:
        using Self = RemotePosterOpenError;
        using Base =
            InternalRemotePosterOpenError<std::exception, sizeof(std::exception) == sizeof(void *)>;

    public:
        RemotePosterOpenError() noexcept = default;

        explicit RemotePosterOpenError(const char *msg) noexcept
            : Base(msg)
        {
        }

        RemotePosterOpenError(const Self &other) noexcept = default;

        RemotePosterOpenError(Self &&other) noexcept = default;

        ~RemotePosterOpenError() noexcept = default;

        Self &operator=(const Self &other) noexcept = default;

        Self &operator=(Self &&other) noexcept = default;
    };
}   // namespace sharpen

#endif