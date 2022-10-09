#pragma once
#ifndef _SHARPEN_IRPCPROVIDER_HPP
#define _SHARPEN_IRPCPROVIDER_HPP

#include "IRemoteProcedure.hpp"

namespace sharpen
{
    class IRpcProvider
    {
    private:
        using Self = sharpen::IRpcProvider;
    protected:
    public:
    
        IRpcProvider() noexcept = default;
    
        IRpcProvider(const Self &other) noexcept = default;
    
        IRpcProvider(Self &&other) noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    
        virtual ~IRpcProvider() noexcept = default;

        virtual void Register(const char *procedureName,std::unique_ptr<sharpen::IRemoteProcedure> procedure) = 0;

        virtual sharpen::IRemoteProcedure *Get(const char *procedureName) noexcept = 0;
    };   
}

#endif