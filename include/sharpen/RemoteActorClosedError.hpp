#pragma once
#ifndef _SHARPEN_ACTORCLOSEDERROR_HPP
#define _SHARPEN_ACTORCLOSEDERROR_HPP

#include <stdexcept>

namespace sharpen
{
    class RemoteActorClosedError:public std::logic_error
    {
    private:
    
        using Self = RemoteActorClosedError;
        using Base = std::logic_error;
    public:
    
        RemoteActorClosedError() noexcept = default;
    
        explicit RemoteActorClosedError(const char *msg) noexcept
            :Base(msg)
        {}
    
        RemoteActorClosedError(const Self &other) noexcept = default;
    
        RemoteActorClosedError(Self &&other) noexcept = default;
    
        ~RemoteActorClosedError() noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    };   
}

#endif