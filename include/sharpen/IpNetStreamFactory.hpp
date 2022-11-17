#pragma once
#ifndef _SHARPEN_IPNETSTREAMFACTORY_HPP
#define _SHARPEN_IPNETSTREAMFACTORY_HPP

#include "INetSteamFactory.hpp"
#include "IpEndPoint.hpp"

namespace sharpen
{
    class IpNetStreamFactory:public sharpen::INetSteamFactory
    {
    private:
        using Self = sharpen::IpNetStreamFactory;
    
        sharpen::EventEngine *engine_;
        sharpen::IpEndPoint localEndpoint_;

        virtual sharpen::NetStreamChannelPtr DoProduce() override;
    public:
    
        IpNetStreamFactory(sharpen::EventEngine &engine,sharpen::IpEndPoint endpoint);
    
        IpNetStreamFactory(const Self &other) = default;
    
        IpNetStreamFactory(Self &&other) noexcept
            :engine_(other.engine_)
            ,localEndpoint_(std::move(other.localEndpoint_))
        {
            other.engine_ = nullptr;
        }
    
        inline Self &operator=(const Self &other)
        {
            if(this != std::addressof(other))
            {
                Self tmp{other};
                std::swap(tmp,*this);
            }
            return *this;
        }
    
        inline Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->engine_ = other.engine_;
                this->localEndpoint_ = std::move(other.localEndpoint_);
                other.engine_ = nullptr;
            }
            return *this;
        }
    
        ~IpNetStreamFactory() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }
    };
}

#endif