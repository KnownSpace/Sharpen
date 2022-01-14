#pragma once
#ifndef _SHARPEN_RPCCLIENT_HPP
#define _SHARPEN_RPCCLIENT_HPP

#include <list>
#include <cassert>

#include "INetStreamChannel.hpp"
#include "CompressedPair.hpp"
#include "SpinLock.hpp"
#include "AwaitableFuture.hpp"
#include "Optional.hpp"
#include "RpcConcepts.hpp"

namespace sharpen
{
    template<typename _Request,typename _Encoder,typename _Response,typename _Decoder>
    using RpcClientRequires = sharpen::BoolType<sharpen::IsRpcEncoder<_Request,_Encoder>::Value 
                                            && sharpen::IsRpcDecoder<_Response,_Decoder>::Value
                                            && sharpen::IsRpcMessage<_Request>::Value
                                            && sharpen::IsRpcMessage<_Response>::Value>;

    template<typename _Request,typename _Encoder,typename _Response,typename _Decoder,typename _Checker = void>
    class InternalRpcClient;

    //NOTE:
    //we will send _Request to server
    //and expect to get a _Response as response
    //_Encoder should has ByteBuffer Encode(const _Request &req) function
    //_Decoder should has size_t Decode(const char *data,size_t size) and bool IsCompleted() function
    //all requests will be sended in a order
    //all response should be returned in same order or you may get a wrong message
    template<typename _Request,typename _Encoder,typename _Response,typename _Decoder>
    class InternalRpcClient<_Request,_Encoder,_Response,_Decoder,sharpen::EnableIf<sharpen::RpcClientRequires<_Request,_Encoder,_Response,_Decoder>::Value>>:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        struct Waiter
        {
            sharpen::Future<_Response> *invokeFuture_;
            sharpen::ByteBuffer buf_;
            sharpen::Future<sharpen::Size> writeFuture_;
        };

        using WaiterList = std::list<sharpen::Future<_Response>*>;
        using Lock = sharpen::SpinLock;
        using Pair = sharpen::CompressedPair<_Encoder,_Decoder>;
        using Self = sharpen::InternalRpcClient<_Request,_Encoder,_Response,_Decoder,void>;

        Lock lock_;
        bool startReaded_;
        Pair pair_;
        sharpen::Future<sharpen::Size> readFuture_;
        sharpen::NetStreamChannelPtr conn_;
        sharpen::ByteBuffer readBuf_;
        WaiterList waiters_;
        _Response res_;
        sharpen::Size lastRead_;
        std::shared_ptr<bool> token_;

        _Decoder &Decoder() noexcept
        {
            return this->pair_.Second();
        }

        const _Decoder &Decoder() const noexcept
        {
            return this->pair_.Second();
        }

        _Encoder &Encoder() noexcept
        {
            return this->pair_.First();
        }

        const _Encoder &Encoder() const noexcept
        {
            return this->pair_.First();
        }

        void ReturnResponse()
        {
            if(this->Decoder().IsCompleted())
            {
                this->Decoder().SetCompleted(false);
                sharpen::Future<_Response> *waiter;
                _Response res;
                {
                    std::unique_lock<Lock> lock(this->lock_);
                    if(this->waiters_.empty())
                    {
                        return;
                    }
                    waiter = std::move(this->waiters_.front());
                    this->waiters_.pop_front();
                    std::swap(res,this->res_);
                }
                if(waiter)
                {
                    waiter->Complete(std::move(res));
                }
            }
        }

        void DealWithData()
        {
            if(this->lastRead_ != 0)
            {
                sharpen::Size size = this->Decoder().Decode(this->readBuf_.Data() + this->readBuf_.GetMark(),this->lastRead_ - this->readBuf_.GetMark());
                this->readBuf_.Mark(this->readBuf_.GetMark() + size);
                if(this->readBuf_.GetMark() == this->lastRead_)
                {
                    this->readBuf_.Mark(0);
                    this->lastRead_ = 0;
                }
                this->ReturnResponse();
            }
        }

        void DealWithError(std::exception_ptr err)
        {
            WaiterList waiters;
            {
                std::unique_lock<Lock> lock(this->lock_);
                waiters.assign(std::make_move_iterator(this->waiters_.begin()),std::make_move_iterator(this->waiters_.end()));
                this->waiters_.erase(this->waiters_.begin(),this->waiters_.end());
                this->startReaded_ = false;
            }
            for (auto begin = waiters.begin(),end = waiters.end(); begin != end; ++begin)
            {
                if(*begin)
                {
                    (*begin)->Fail(err);
                }   
            }
        }

        void ReadCallback(std::shared_ptr<bool> token,sharpen::Future<sharpen::Size> &readFuture)
        {
            if(!*token)
            {
                return;
            }
            if(readFuture.IsError())
            {
                this->DealWithError(readFuture.Error());
                return;
            }
            sharpen::Size size = readFuture.Get();
            if(size == 0)
            {
                //disconnect
                this->DealWithError(sharpen::MakeSystemErrorPtr(sharpen::ErrorConnectionAborted));
                return;
            }
            this->lastRead_ = size;
            this->DealWithData();
            this->StartRead();
        }

        void StartRead()
        {
            bool need{false};
            {
                std::unique_lock<Lock> lock(this->lock_);
                need = !this->waiters_.empty();
                if(!need)
                {
                    this->startReaded_ = false;
                    return;
                }
                this->Decoder().Bind(this->res_);
            }
            if(need)
            {
                this->readFuture_.Reset();
                this->readFuture_.SetCallback(std::bind(&Self::ReadCallback,this,this->token_,std::placeholders::_1));
                this->conn_->ReadAsync(this->readBuf_,0,this->readFuture_);
            }
        }

        void WriteRequestCallback(Waiter *waiter,sharpen::Future<sharpen::Size> &writeFuture)
        {
            std::unique_ptr<Waiter> wp{waiter};
            if(writeFuture.IsError())
            {
                if(wp->invokeFuture_)
                {
                    wp->invokeFuture_->Fail(std::move(writeFuture.Error()));
                }
                return;
            }
            bool started{true};
            {
                std::unique_lock<Lock> lock(this->lock_);
                this->waiters_.push_back(std::move(wp->invokeFuture_));
                std::swap(started,this->startReaded_);
            }
            if(!started)
            {
                this->DealWithData();
                this->StartRead();
            }
        }
    public:
        explicit InternalRpcClient(sharpen::NetStreamChannelPtr conn)
            :InternalRpcClient(conn,_Encoder{},_Decoder{})
        {}

        InternalRpcClient(sharpen::NetStreamChannelPtr conn,_Encoder encoder)
            :InternalRpcClient(conn,std::move(encoder),_Decoder{})
        {}

        InternalRpcClient(sharpen::NetStreamChannelPtr conn,_Encoder encoder,_Decoder decoder)
            :lock_()
            ,startReaded_(false)
            ,pair_()
            ,readFuture_()
            ,conn_(std::move(conn))
            ,readBuf_(4096)
            ,waiters_()
            ,res_()
            ,lastRead_(0)
            ,token_(std::make_shared<bool>(true))
        {
            this->pair_.First() = std::move(encoder);
            this->pair_.Second() = std::move(decoder);
        }

        virtual ~InternalRpcClient() noexcept = default;

        //send a package to rpc server
        //and receive a return package
        void InvokeAsync(sharpen::Future<_Response> &future,const _Request &package)
        {
            Waiter *waiter = new Waiter{};
            if(!waiter)
            {
                throw std::bad_alloc();
            }
            waiter->invokeFuture_ = &future;
            this->Encoder().EncodeTo(package,waiter->buf_);
            waiter->writeFuture_.SetCallback(std::bind(&Self::WriteRequestCallback,this,waiter,std::placeholders::_1));
            this->conn_->WriteAsync(waiter->buf_,0,waiter->writeFuture_);
        }

        _Response InvokeAsync(const _Request &package)
        {
            sharpen::AwaitableFuture<_Response> invoker;
            this->InvokeAsync(invoker,package);
            return std::move(invoker.Await());
        }

        void Cancel() noexcept
        {
            this->conn_->Cancel();
        }
    };

    template<typename _Request,typename _Encoder,typename _Response,typename _Decoder>
    using RpcClient = sharpen::InternalRpcClient<_Request,_Encoder,_Response,_Decoder>;
}

#endif