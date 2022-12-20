#pragma once
#ifndef _SHARPEN_IPNETSTREAMFACTORY_HPP
#define _SHARPEN_IPNETSTREAMFACTORY_HPP

#include "ITcpSteamFactory.hpp"
#include "IpEndPoint.hpp"
#include "IEventLoopGroup.hpp"

namespace sharpen
{
    class IpTcpStreamFactory:public sharpen::ITcpSteamFactory
    {
    private:
        using Self = sharpen::IpTcpStreamFactory;
    
        sharpen::IEventLoopGroup *loopGroup_;
        sharpen::IpEndPoint localEndpoint_;

        virtual sharpen::NetStreamChannelPtr NviProduce() override;
    public:
    
        IpTcpStreamFactory(sharpen::IEventLoopGroup &loopGroup,sharpen::IpEndPoint endpoint);
    
        IpTcpStreamFactory(const Self &other) = default;
    
        IpTcpStreamFactory(Self &&other) noexcept
            :loopGroup_(other.loopGroup_)
            ,localEndpoint_(std::move(other.localEndpoint_))
        {
            other.loopGroup_ = nullptr;
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
                this->loopGroup_ = other.loopGroup_;
                this->localEndpoint_ = std::move(other.localEndpoint_);
                other.loopGroup_ = nullptr;
            }
            return *this;
        }
    
        ~IpTcpStreamFactory() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }
    };
}

#endif