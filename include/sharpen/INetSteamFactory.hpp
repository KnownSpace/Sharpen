#pragma once
#ifndef _SHARPEN_INETSTEAMFACTORY_HPP
#define _SHARPEN_INETSTEAMFACTORY_HPP

#include "INetStreamChannel.hpp"

namespace sharpen
{
    class INetSteamFactory
    {
    private:
        using Self = sharpen::INetSteamFactory;

    protected:

        virtual sharpen::NetStreamChannelPtr DoProduce() = 0;
    public:
    
        INetSteamFactory() noexcept = default;
    
        INetSteamFactory(const Self &other) noexcept = default;
    
        INetSteamFactory(Self &&other) noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    
        virtual ~INetSteamFactory() noexcept = default;
    
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