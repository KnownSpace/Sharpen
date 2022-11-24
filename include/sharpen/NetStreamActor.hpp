#pragma once
#ifndef _SHARPEN_NETSTREAMACTOR_HPP
#define _SHARPEN_NETSTREAMACTOR_HPP

#include "IRemoteActor.hpp"
#include "IEndPoint.hpp"
#include "INetSteamFactory.hpp"
#include "RemoteActorClosedError.hpp"
#include "IMailParser.hpp"

namespace sharpen
{
    class NetStreamActor:public sharpen::IRemoteActor,public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::NetStreamActor;
    
        std::unique_ptr<sharpen::SpinLock> lock_;
        sharpen::NetStreamChannelPtr channel_;
        std::unique_ptr<sharpen::IEndPoint> remoteEndpoint_;
        std::unique_ptr<sharpen::IMailParser> parser_;
        sharpen::INetSteamFactory *factory_;

        virtual std::uint64_t DoGetId() const noexcept override;

        virtual sharpen::Mail DoPost(const sharpen::Mail &mail) override;

        virtual void DoClose() noexcept override;

        virtual void DoOpen() override;
    public:
    
        NetStreamActor(std::unique_ptr<sharpen::IEndPoint> endpoint,std::unique_ptr<sharpen::IMailParser> parser,sharpen::INetSteamFactory *factory);
        
        NetStreamActor(Self &&other) noexcept;

        virtual ~NetStreamActor() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        Self &operator=(Self &&other) noexcept;
    };
}

#endif