#pragma once
#ifndef _SHARPEN_RPCSERVER_HPP
#define _SHARPEN_RPCSERVER_HPP

#include <unordered_map>
#include <functional>
#include <string>

#include "CompressedPair.hpp"
#include "TcpServer.hpp"
#include "RpcContext.hpp"
#include "RpcConcepts.hpp"
#include "RpcServerOption.hpp"
#include "ITimer.hpp"

namespace sharpen
{

    template<typename _Response,typename _Encoder,typename _Request,typename _Decoder,typename _Dispatcher>
    using RpcServerRequires = sharpen::BoolType<sharpen::IsRpcEncoder<_Response,_Encoder>::Value
                                                && sharpen::IsRpcDecoder<_Request,_Decoder>::Value
                                                && sharpen::IsRpcMessage<_Request>::Value
                                                && sharpen::IsRpcMessage<_Response>::Value
                                                && sharpen::IsRpcDispatcher<_Request,_Dispatcher>::Value>;

    template<typename _Response,typename _Encoder,typename _Request,typename _Decoder,typename _Dispatcher,typename _Data = void,typename _Check = void>
    class InternalRpcServer;

    //Note:
    //we will get a _Request and pass it to _Dispatcher
    //forward it to a handler
    //_Encoder should has ByteBuffer Encode(_Response res) function
    //_Request should has void Clear() function
    //_Decoder should has size_t Decode(const char *data,size_t size) , bool IsCompleted() and void SetCompleted(bool completed) function
    //_Dispatcher should has std::string GetProcedureName(const _Request &req) function
    template<typename _Response,typename _Encoder,typename _Request,typename _Decoder,typename _Dispatcher,typename _Data>
    class InternalRpcServer<_Response,_Encoder,_Request,_Decoder,_Dispatcher,_Data,sharpen::EnableIf<sharpen::RpcServerRequires<_Response,_Encoder,_Request,_Decoder,_Dispatcher>::Value>>:public sharpen::TcpServer
    {
    private:
        using Base = sharpen::TcpServer;
    public:
        using Context = sharpen::RpcContext<_Encoder,_Request,_Decoder,_Data>;
    private:
        using Handler = std::function<void(Context&)>;
        using Map = std::unordered_map<std::string,Handler>;
        using Pair = sharpen::CompressedPair<Map,_Dispatcher>;
        using EncoderBuilder = std::function<_Encoder()>;
        using DecoderBuilder = std::function<_Decoder()>;

        Pair pair_;
        sharpen::Option<std::chrono::milliseconds> timeout_;
        DecoderBuilder decoderBuilder_;
        EncoderBuilder encoderBuilder_;
        Handler timeoutHandler_;

        Map &GetMap() noexcept
        {
            return this->pair_.First();
        }

        const Map &GetMap() const noexcept
        {
            return this->pair_.First();
        }

        _Dispatcher &GetDispatcher() noexcept
        {
            return this->pair_.Second();
        }

        const _Dispatcher &GetDispatcher() const noexcept
        {
            return this->pair_.Second();
        }

        static void TimeoutCallback(sharpen::Future<void> &,sharpen::Future<sharpen::Size> *future,Context *ctx)
        {
            if(future->IsPending())
            {
                ctx->Connection()->Cancel();
            }
        }
    protected:
        virtual void OnNewChannel(sharpen::NetStreamChannelPtr channel) override
        {
            Context ctx(std::move(channel),this->encoderBuilder_ ? this->encoderBuilder_():_Encoder{},this->decoderBuilder_? this->decoderBuilder_():_Decoder{});
            sharpen::ByteBuffer buf{4096};
            sharpen::TimerPtr timer;
            sharpen::AwaitableFuture<void> timeout;
            sharpen::AwaitableFuture<sharpen::Size> future;
            if(this->timeout_.HasValue())
            {
                timer = sharpen::MakeTimer(*this->engine_);
            }
            while (true)
            {
                ctx.Request().Clear();
                ctx.Decoder().SetCompleted(false);
                sharpen::Size lastSize{0};
                while (!ctx.Decoder().IsCompleted())
                {                  
                    sharpen::Size size{0};
                    if(buf.GetMark() == 0)
                    {
                        future.Reset();
                        //timeout model
                        if(this->timeout_.HasValue())
                        {
                            //init
                            timeout.Reset();
                            //begin request
                            timer->WaitAsync(timeout,this->timeout_.Get());
                            using FnPtr = void(*)(sharpen::Future<void>&,sharpen::Future<sharpen::Size>*,Context*);
                            timeout.SetCallback(std::bind(static_cast<FnPtr>(&TimeoutCallback),std::placeholders::_1,&future,&ctx));
                            ctx.Connection()->ReadAsync(buf,0,future);
                            try
                            {
                                size = future.Await();
                                timer->Cancel();
                            }
                            catch(const std::exception&)
                            {
                                //socket error
                                if(timeout.IsPending())
                                {
                                    timer->Cancel();
                                    return;
                                }
                                //timeout
                                if(this->timeoutHandler_)
                                {
                                    this->timeoutHandler_(ctx);
                                    return;
                                }
                            }
                        }
                        else
                        {
                            ctx.Connection()->ReadAsync(buf,0,future);
                            size = future.Await();
                        }
                        if(size == 0)
                        {
                            return;
                        }
                        lastSize = size;
                    }
                    else
                    {
                        size = lastSize - buf.GetMark();
                    }
                    sharpen::Size dSize = ctx.Decoder().Decode(buf.Data() + buf.GetMark(),size);
                    if(dSize != size)
                    {
                        buf.Mark(buf.GetMark() + dSize);
                    }
                    else
                    {
                        buf.Mark(0);
                    }
                }
                _Dispatcher &dispatcher = this->GetDispatcher();
                const std::string &name = dispatcher.GetProcedureName(ctx.Request());
                auto ite = this->GetMap().find(name);
                if(ite != this->GetMap().end())
                {
                    ite->second(ctx);
                }   
            }
        }
    public:
        using Option = sharpen::RpcServerOption<_Encoder,_Decoder,_Dispatcher>;

        InternalRpcServer(sharpen::AddressFamily af,const sharpen::IEndPoint &endpoint,sharpen::EventEngine &engine,Option option)
            :Base(af,endpoint,engine)
            ,pair_()
            ,timeout_(std::move(option.TimeoutOption()))
            ,encoderBuilder_(std::move(option.GetEncoderBuilder()))
            ,decoderBuilder_(std::move(option.GetDecoderBuilder()))
        {
            this->pair_.Second() = std::move(option.Dispatcher());
        }

        void Register(const std::string &name,Handler handler)
        {
            this->GetMap()[name] = std::move(handler);
        }

        void RegisterTimeout(Handler handler)
        {
            this->timeoutHandler_ = std::move(handler);
        }

        virtual ~InternalRpcServer() noexcept = default;
    };

    template<typename _Response,typename _Encoder,typename _Request,typename _Decoder,typename _Dispatcher,typename _Data = void>
    using RpcServer = sharpen::InternalRpcServer<_Response,_Encoder,_Request,_Decoder,_Dispatcher,_Data>;
}

#endif