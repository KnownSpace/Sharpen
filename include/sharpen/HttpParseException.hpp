#pragma once
#ifndef _SHARPEN_HTTPPARSEEXCEPTION_HPP
#define _SHARPEN_HTTPPARSEEXCEPTION_HPP

#include <stdexcept>

namespace sharpen
{
    class HttpParseException:public std::logic_error
    {
    private:
        using Base = std::logic_error;
        using Self = sharpen::HttpParseException;

    public:
        explicit HttpParseException(const char *message) noexcept
            :Base(message)
        {}

        HttpParseException(const Self &other) noexcept = default;

        HttpParseException(Self &&other) noexcept = default;

        ~HttpParseException() noexcept = default;

        Self &operator=(const Self &other) noexcept = default;

        Self &operator=(Self &&other) noexcept = default;
    };
}

#endif