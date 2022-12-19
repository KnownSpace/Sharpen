#pragma once
#ifndef _SHARPEN_INETSTEAMFACTORY_HPP
#define _SHARPEN_INETSTEAMFACTORY_HPP

#include "INetStreamChannel.hpp"

namespace sharpen
{
    class ITcpSteamFactory
    {
    private:
        using Self = sharpen::ITcpSteamFactory;

    protected:

        virtual sharpen::NetStreamChannelPtr DoProduce() = 0;
    public:
    
        ITcpSteamFactory() noexcept = default;
    
        ITcpSteamFactory(const Self &other) noexcept = default;
    
        ITcpSteamFactory(Self &&other) noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    
        virtual ~ITcpSteamFactory() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline sharpen::NetStreamChannelPtr Produce()
        {
            return this->DoProduce();
        }
    };
}

#endif