#pragma once
#ifndef _SHARPEN_NETSTREAMACTOR_HPP
#define _SHARPEN_NETSTREAMACTOR_HPP

#include "IRemoteActor.hpp"
#include "IEndPoint.hpp"
#include "INetSteamFactory.hpp"
#include "ActorClosedError.hpp"

namespace sharpen
{
    class NetStreamActor:public sharpen::IRemoteActor,public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Self = sharpen::NetStreamActor;
    
        sharpen::SpinLock lock_;
        sharpen::NetStreamChannelPtr channel_;
        std::unique_ptr<sharpen::IEndPoint> remoteEndpoint_;
        sharpen::INetSteamFactory *factory_;

        virtual std::uint64_t DoGetAddressHash() const noexcept override;

        virtual std::unique_ptr<sharpen::IMail> DoPost(const sharpen::IMail &mail) override;

        virtual void DoClose() noexcept override;

        virtual void DoOpen() override;

    protected:

        virtual std::unique_ptr<sharpen::IMail> ReceiveMail(sharpen::INetStreamChannel *channel) = 0;
    public:
    
        NetStreamActor(std::unique_ptr<sharpen::IEndPoint> endpoint,sharpen::INetSteamFactory *factory);
        
        virtual ~NetStreamActor() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }
    };
}

#endif