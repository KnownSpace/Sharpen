#pragma once
#ifndef _SHARPEN_MAILPARSEERROR_HPP
#define _SHARPEN_MAILPARSEERROR_HPP

#include <exception>

namespace sharpen
{
    template<typename _Exception, bool _IsEmpty>
    class InternalMailParseError : public _Exception
    {
    private:
        using Self = sharpen::InternalMailParseError<_Exception, _IsEmpty>;

        const char *msg_;

    public:
        InternalMailParseError() noexcept = default;

        explicit InternalMailParseError(const char *msg)
            : msg_(msg)
        {
        }

        InternalMailParseError(const Self &other) noexcept = default;

        InternalMailParseError(Self &&other) noexcept = default;

        ~InternalMailParseError() noexcept = default;

        Self &operator=(const Self &other) noexcept = default;

        Self &operator=(Self &&other) noexcept = default;

        virtual const char *what() const noexcept override
        {
            return this->msg_;
        }
    };

    // msvc exception
    template<typename _Exception>
    class InternalMailParseError<_Exception, false> : public _Exception
    {
    private:
        using Self = sharpen::InternalMailParseError<_Exception, false>;
        using Base = _Exception;

    public:
        InternalMailParseError() noexcept = default;

        explicit InternalMailParseError(const char *msg) noexcept
            : Base(msg)
        {
        }

        InternalMailParseError(const Self &other) noexcept = default;

        InternalMailParseError(Self &&other) noexcept = default;

        ~InternalMailParseError() noexcept = default;

        Self &operator=(const Self &other) noexcept = default;

        Self &operator=(Self &&other) noexcept = default;
    };

    // if sizeof(std::exception) == sizeof(void*),std::exception is a interface class
    // exception class defination
    class MailParseError
        : public InternalMailParseError<std::exception, sizeof(std::exception) == sizeof(void *)>
    {
    private:
        using Self = sharpen::MailParseError;
        using Base = sharpen::InternalMailParseError<std::exception,
                                                     sizeof(std::exception) == sizeof(void *)>;

    public:
        MailParseError() noexcept = default;

        explicit MailParseError(const char *msg) noexcept
            : Base(msg)
        {
        }

        MailParseError(const Self &other) noexcept = default;

        MailParseError(Self &&other) noexcept = default;

        ~MailParseError() noexcept = default;

        Self &operator=(const Self &other) noexcept = default;

        Self &operator=(Self &&other) noexcept = default;
    };
}   // namespace sharpen

#endif