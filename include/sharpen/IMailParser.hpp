#pragma once
#ifndef _SHARPEN_IMAILPARSER_HPP
#define _SHARPEN_IMAILPARSER_HPP

#include <memory>

#include "Mail.hpp"
#include "ByteSlice.hpp"

namespace sharpen
{
    class IMailParser
    {
    private:
        using Self = sharpen::IMailParser;
    protected:
    public:
    
        IMailParser() noexcept = default;
    
        IMailParser(const Self &other) noexcept = default;
    
        IMailParser(Self &&other) noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    
        virtual ~IMailParser() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        virtual void Parse(const sharpen::ByteSlice &slice) = 0;

        virtual sharpen::Mail PopCompletedMail() = 0;

        virtual bool Completed() const noexcept = 0;
    };
}

#endif