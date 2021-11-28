#pragma once
#ifndef _SHARPEN_MICRORPCPARSEEXCEPTION_HPP
#define _SHARPEN_MICRORPCPARSEEXCEPTION_HPP

#include <stdexcept>

namespace sharpen
{
    class MicroRpcParseException:public std::logic_error
    {
    private:
        using Base = std::logic_error;
        using Self = sharpen::MicroRpcParseException;

    public:
        explicit MicroRpcParseException(const char *message) noexcept
            :Base(message)
        {}

        MicroRpcParseException(const Self &other) noexcept = default;

        MicroRpcParseException(Self &&other) noexcept = default;

        ~MicroRpcParseException() noexcept = default;

        Self &operator=(const Self &other) noexcept = default;

        Self &operator=(Self &&other) noexcept = default;
    };
}

#endif