#pragma once
#ifndef _SHARPEN_NETSTREAMACTOR_HPP
#define _SHARPEN_NETSTREAMACTOR_HPP

#include "IRemotePoster.hpp"
#include "IEndPoint.hpp"
#include "ITcpSteamFactory.hpp"
#include "RemotePosterClosedError.hpp"
#include "RemotePosterOpenError.hpp"
#include "IMailParser.hpp"
#include "Noncopyable.hpp"

namespace sharpen
{
    class TcpPoster:public sharpen::IRemotePoster,public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::TcpPoster;
    
        std::unique_ptr<sharpen::SpinLock> lock_;
        sharpen::NetStreamChannelPtr channel_;
        std::unique_ptr<sharpen::IEndPoint> remoteEndpoint_;
        std::unique_ptr<sharpen::IMailParser> parser_;
        sharpen::ITcpSteamFactory *factory_;

        virtual std::uint64_t DoGetId() const noexcept override;

        virtual sharpen::Mail DoPost(const sharpen::Mail &mail) override;

        virtual void DoClose() noexcept override;

        virtual void DoOpen() override;
    public:
    
        TcpPoster(std::unique_ptr<sharpen::IEndPoint> endpoint,std::unique_ptr<sharpen::IMailParser> parser,sharpen::ITcpSteamFactory *factory);
        
        TcpPoster(Self &&other) noexcept;

        virtual ~TcpPoster() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        Self &operator=(Self &&other) noexcept;
    };
}

#endif