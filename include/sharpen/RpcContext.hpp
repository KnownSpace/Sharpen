#pragma once
#ifndef _SHARPEN_RPCCONTEXT_HPP
#define _SHARPEN_RPCCONTEXT_HPP

#include "INetStreamChannel.hpp"
#include "CompressedPair.hpp"

namespace sharpen
{
    template<typename _Encoder,typename _Request,typename _Decoder>
    class RpcContext:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Pair = sharpen::CompressedPair<_Encoder,_Decoder>;
        using Self = sharpen::RpcContext<_Encoder,_Request,_Decoder>;

        sharpen::NetStreamChannelPtr conn_;
        _Request req_;
        Pair pair_;
    public:
        explicit RpcContext(sharpen::NetStreamChannelPtr conn)
            :RpcContext(std::move(conn),_Encoder{},_Decoder{})
        {}

        RpcContext(sharpen::NetStreamChannelPtr conn,_Encoder encoder)
            :RpcContext(std::move(conn),std::move(encoder),_Decoder{})
        {}

        RpcContext(sharpen::NetStreamChannelPtr conn,_Encoder encoder,_Decoder decoder)
            :conn_(std::move(conn))
            ,req_()
            ,pair_()
        {
            this->pair_.First() = std::move(encoder);
            this->pair_.Second() = std::move(decoder);
            this->Decoder().Bind(this->req_);
        }

        ~RpcContext() noexcept = default;

        sharpen::NetStreamChannelPtr &Connection() noexcept
        {
            return this->conn_;
        }

        const sharpen::NetStreamChannelPtr &Connection() const noexcept
        {
            return this->conn_;
        }

        _Request &Request() noexcept
        {
            return this->req_;
        }

        const _Request &Request() const noexcept
        {
            return this->req_;
        }

        _Encoder &Encoder() noexcept
        {
            return this->pair_.First();
        }

        const _Encoder &Encoder() const noexcept
        {
            return this->pair_.First();
        }

        _Decoder &Decoder() noexcept
        {
            return this->pair_.Second();
        }

        const _Decoder &Decoder() const noexcept
        {
            return this->pair_.Second();
        }
    };
}

#endif