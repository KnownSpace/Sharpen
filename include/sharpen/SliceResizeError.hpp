#pragma once
#ifndef _SHARPEN_SLICERESIZEERROR_HPP
#define _SHARPEN_SLICERESIZEERROR_HPP

#include <stdexcept>

namespace sharpen
{
    class SliceResizeError:public std::logic_error
    {
    private:
    
        using Self = SliceResizeError;
        using Base = std::logic_error;
    public:
    
        SliceResizeError() noexcept
            :Base("slice cannot resize")
        {}
    
        explicit SliceResizeError(const char *msg) noexcept
            :Base(msg)
        {}
    
        SliceResizeError(const Self &other) noexcept = default;
    
        SliceResizeError(Self &&other) noexcept = default;
    
        ~SliceResizeError() noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    };   
}

#endif