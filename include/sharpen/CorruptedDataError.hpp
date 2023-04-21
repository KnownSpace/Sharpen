#pragma once
#ifndef _SHARPEN_DATACORRUPTIONEXCEPTION_HPP
#define _SHARPEN_DATACORRUPTIONEXCEPTION_HPP

#include <exception>

namespace sharpen
{
    template<typename _Exception, bool _IsEmpty>
    class InternalCorruptedDataError : public _Exception
    {
    private:
        using Self = sharpen::InternalCorruptedDataError<_Exception, _IsEmpty>;

        const char *msg_;

    public:
        InternalCorruptedDataError() noexcept = default;

        explicit InternalCorruptedDataError(const char *msg)
            : msg_(msg)
        {
        }

        InternalCorruptedDataError(const Self &other) noexcept = default;

        InternalCorruptedDataError(Self &&other) noexcept = default;

        ~InternalCorruptedDataError() noexcept = default;

        Self &operator=(const Self &other) noexcept = default;

        Self &operator=(Self &&other) noexcept = default;

        virtual const char *what() const noexcept override
        {
            return this->msg_;
        }
    };

    // msvc exception
    template<typename _Exception>
    class InternalCorruptedDataError<_Exception, false> : public _Exception
    {
    private:
        using Self = sharpen::InternalCorruptedDataError<_Exception, false>;
        using Base = _Exception;

    public:
        InternalCorruptedDataError() noexcept = default;

        explicit InternalCorruptedDataError(const char *msg) noexcept
            : Base(msg)
        {
        }

        InternalCorruptedDataError(const Self &other) noexcept = default;

        InternalCorruptedDataError(Self &&other) noexcept = default;

        ~InternalCorruptedDataError() noexcept = default;

        Self &operator=(const Self &other) noexcept = default;

        Self &operator=(Self &&other) noexcept = default;
    };

    // if sizeof(std::exception) == sizeof(void*),std::exception is a interface class
    // exception class defination
    class CorruptedDataError
        : public sharpen::InternalCorruptedDataError<std::exception,
                                                     sizeof(std::exception) == sizeof(void *)>
    {
    private:
        using Self = sharpen::CorruptedDataError;
        using Base = sharpen::InternalCorruptedDataError<std::exception,
                                                         sizeof(std::exception) == sizeof(void *)>;

    public:
        CorruptedDataError() noexcept = default;

        explicit CorruptedDataError(const char *msg) noexcept
            : Base(msg)
        {
        }

        CorruptedDataError(const Self &other) noexcept = default;

        CorruptedDataError(Self &&other) noexcept = default;

        ~CorruptedDataError() noexcept = default;

        Self &operator=(const Self &other) noexcept = default;

        Self &operator=(Self &&other) noexcept = default;
    };
}   // namespace sharpen

#endif