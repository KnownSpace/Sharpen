#pragma once
#ifndef _SHARPEN_IMAIL_HPP
#define _SHARPEN_IMAIL_HPP

#include <cstdint>
#include <cstddef>

#include "IMailContent.hpp"

namespace sharpen
{
    class IMail
    {
    private:
        using Self = sharpen::IMail;
    protected:
    public:
    
        IMail() noexcept = default;
    
        IMail(const Self &other) noexcept = default;
    
        IMail(Self &&other) noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    
        virtual ~IMail() noexcept = default;

        inline const Self &Const() const noexcept
        {
            return *this;
        }
    
        virtual std::uint64_t GetSourceHash() const noexcept = 0;

        virtual sharpen::IMailContent &Content() noexcept;

        virtual const sharpen::IMailContent &Content() const noexcept;
    };
}

#endif