#pragma once
#ifndef _SHARPEN_TCPACCEPTOR_HPP
#define _SHARPEN_TCPACCEPTOR_HPP

#include "INetStreamChannel.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "EventEngine.hpp"

namespace sharpen
{
    class TcpAcceptor:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:

        sharpen::NetStreamChannelPtr listener_;
    public:
        explicit TcpAcceptor(sharpen::AddressFamily af,const sharpen::IEndPoint &endpoint,sharpen::EventEngine &engine);

        ~TcpAcceptor() noexcept = default;

        sharpen::NetStreamChannelPtr AcceptAsync();

        inline void GetLocalEndPoint(sharpen::IEndPoint &endpoint) const
        {
            this->listener_->GetLocalEndPoint(endpoint);
        }

        inline void GetRemoteEndPoint(sharpen::IEndPoint &endpoint) const
        {
            this->listener_->GetRemoteEndPoint(endpoint);
        }

        inline void Close()
        {
            this->listener_->Cancel();
        }
    };
}

#endif