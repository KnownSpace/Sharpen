#pragma once
#ifndef _SHARPEN_ACTORCLOSEDERROR_HPP
#define _SHARPEN_ACTORCLOSEDERROR_HPP

#include <stdexcept>

namespace sharpen
{
    class ActorClosedError:public std::logic_error
    {
    private:
    
        using Self = ActorClosedError;
        using Base = std::logic_error;
    public:
    
        ActorClosedError() noexcept = default;
    
        explicit ActorClosedError(const char *msg) noexcept
            :Base(msg)
        {}
    
        ActorClosedError(const Self &other) noexcept = default;
    
        ActorClosedError(Self &&other) noexcept = default;
    
        ~ActorClosedError() noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    };   
}

#endif