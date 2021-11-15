#pragma once
#ifndef _SHARPEN_RPCCLIENT_HPP
#define _SHARPEN_RPCCLIENT_HPP

#include <list>
#include <cassert>

#include "INetStreamChannel.hpp"
#include "CompressedPair.hpp"
#include "SpinLock.hpp"
#include "AwaitableFuture.hpp"
#include "Option.hpp"
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
            sharpen::Option<sharpen::Future<sharpen::Size>> sender_;
            sharpen::Future<_Response> *invoker_;
        };

        using WaiterList = std::list<Waiter>;
        using Reader = sharpen::Future<sharpen::Size>;
        using Lock = sharpen::SpinLock;
        using Pair = sharpen::CompressedPair<_Encoder,_Decoder>;
        using Self = sharpen::InternalRpcClient<_Request,_Encoder,_Response,_Decoder,void>;

        Lock lock_;
        bool started_;
        Pair pair_;
        Reader reader_;
        sharpen::NetStreamChannelPtr conn_;
        sharpen::ByteBuffer readBuf_;
        WaiterList waiters_;
        _Response res_;
        sharpen::Size lastRead_;

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

        void StartReceive()
        {
            this->reader_.Reset();
            this->res_.ConfigDecoder(this->Decoder());
            if (this->lastRead_ && this->readBuf_.GetMark() != 0)
            {
                HandleData(this->lastRead_);
                HandleCallback();
                return;
            }
            this->reader_.SetCallback(std::bind(&Self::ReceiveCallback,this,std::placeholders::_1));
            this->conn_->ReadAsync(this->readBuf_,0,this->reader_);
        }

        void FailReceive(std::exception_ptr err)
        {
            WaiterList waiters;
            {
                std::unique_lock<Lock> lock(this->lock_);
                auto begin = this->waiters_.begin(),end = this->waiters_.end();
                for (;begin != end;++begin)
                {
                    if(begin->sender_.Get().IsPending())
                    {
                        break;
                    }
                }
                waiters.assign(std::make_move_iterator(begin),std::make_move_iterator(end));
                this->waiters_.erase(begin,end);
                this->started_ = false;
            }
            for (auto begin = waiters.begin(),end = waiters.end(); begin != end; begin++)
            {
                begin->invoker_->Fail(err);   
            }
        }

        void HandleCallback()
        {
            bool complete = this->Decoder().IsCompleted();
            this->Decoder().SetCompleted(false);
            Waiter last;
            last.invoker_ = nullptr;
            _Response res;
            if(complete)
            {
                res = std::move(this->res_);
                //notify future
                std::unique_lock<Lock> lock(this->lock_);
                assert(!this->waiters_.empty());
                last = std::move(this->waiters_.front());
                this->waiters_.pop_front();
                if(!this->waiters_.empty())
                {
                    //continue request
                    this->StartReceive();
                }
                else
                {
                    this->started_ = false;
                }
            }
            if(last.invoker_)
            {
                last.invoker_->Complete(std::move(res));
            }
        }

        void HandleData(sharpen::Size size)
        {
            size -= this->readBuf_.GetMark();
            //try decode message
            sharpen::Size dSize = this->Decoder().Decode(this->readBuf_.Data() + this->readBuf_.GetMark(),size);
            //more than a request
            if(dSize != size)
            {
                //move mark
                this->readBuf_.Mark(dSize + this->readBuf_.GetMark());
            }
            else
            {
                //set mark to 0
                this->readBuf_.Mark(0);
            }
        }

        void ReceiveCallback(sharpen::Future<sharpen::Size> &future)
        {
            try
            {
                this->lastRead_ = future.Get();
                if(this->lastRead_ == 0)
                {
                    sharpen::ThrowSystemError(sharpen::ErrorConnectionAborted);
                }
                this->HandleData(this->lastRead_);
            }
            catch(const std::exception &)
            {
                this->FailReceive(std::current_exception());
                return;
            }
            this->HandleCallback();
        }

        void SendCallback(sharpen::Future<_Response> &invoker,sharpen::Future<sharpen::Size> &future)
        {
            try
            {
                future.Get();
            }
            catch(const std::exception&)
            {
                invoker.Fail(std::current_exception());
                return;
            }
            bool started{true};
            {
                std::unique_lock<Lock> lock(this->lock_);
                std::swap(started,this->started_);
            }
            if(!started)
            {
                this->StartReceive();
            }
        }

        void DoCancel(sharpen::ErrorCode err) noexcept
        {
            this->conn_->Cancel();
            this->FailReceive(sharpen::MakeSystemErrorPtr(err));
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
            ,started_(false)
            ,pair_()
            ,reader_()
            ,conn_(std::move(conn))
            ,readBuf_(4096)
            ,waiters_()
            ,res_()
            ,lastRead_(0)
        {
            this->pair_.First() = std::move(encoder);
            this->pair_.Second() = std::move(decoder);
        }

        virtual ~InternalRpcClient() noexcept
        {
            this->conn_->Close();
            this->DoCancel(sharpen::ErrorConnectionAborted);
        }

        //send a package to rpc server
        //and receive a return package
        void InvokeAsync(sharpen::Future<_Response> &future,const _Request &package)
        {
            Waiter waiter;
            waiter.invoker_ = &future;
            waiter.sender_.Construct();
            Waiter *waiterPtr = nullptr;
            {
                std::unique_lock<Lock> lock(this->lock_);
                this->waiters_.push_back(std::move(waiter));
                waiterPtr = &this->waiters_.back();
            }
            waiterPtr->sender_.Get().SetCallback(std::bind(&Self::SendCallback,this,std::ref(future),std::placeholders::_1));
            this->conn_->WriteAsync(this->Encoder().Encode(package),0,waiterPtr->sender_.Get());
        }

        _Response InvokeAsync(const _Request &package)
        {
            sharpen::AwaitableFuture<_Response> invoker;
            this->InvokeAsync(invoker,package);
            return invoker.Await();
        }

        void Cancel() noexcept
        {
            this->DoCancel(sharpen::ErrorCancel);
        }
    };

    template<typename _Request,typename _Encoder,typename _Response,typename _Decoder>
    using RpcClient = sharpen::InternalRpcClient<_Request,_Encoder,_Response,_Decoder>;
}

#endif