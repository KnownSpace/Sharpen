#pragma once
#ifndef _SHARPEN_CHECKSUMEXCEPTION_HPP
#define _SHARPEN_CHECKSUMEXCEPTION_HPP

#include <stdexcept>

namespace sharpen
{
    class ChecksumException:public std::logic_error
    {
    private:
        using Base = std::logic_error;
        using Self = sharpen::ChecksumException;

    public:
        explicit ChecksumException(const char *message) noexcept
            :Base(message)
        {}

        ChecksumException(const Self &other) noexcept = default;

        ChecksumException(Self &&other) noexcept = default;

        ~ChecksumException() noexcept = default;

        Self &operator=(const Self &other) noexcept = default;

        Self &operator=(Self &&other) noexcept = default;
    };
}

#endif