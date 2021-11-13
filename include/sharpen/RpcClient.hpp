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

namespace sharpen
{
    template<typename _Request,typename _Encoder,typename _Response,typename _Decoder>
    class RpcClient:public sharpen::Noncopyable,public sharpen::Nonmovable
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
        using Self = sharpen::RpcClient<_Request,_Encoder,_Response,_Decoder>;

        Lock lock_;
        bool started_;
        Pair pair_;
        Reader reader_;
        sharpen::NetStreamChannelPtr conn_;
        sharpen::ByteBuffer readBuf_;
        WaiterList waiters_;
        _Response res_;

        _Decoder &GetDecoder() noexcept
        {
            return this->pair_.Second();
        }

        const _Decoder &GetDecoder() const noexcept
        {
            return this->pair_.Second();
        }

        _Encoder &GetEncoder() noexcept
        {
            return this->pair_.First();
        }

        const _Encoder &GetEncoder() const noexcept
        {
            return this->pair_.First();
        }

        void StartReceive()
        {
            this->reader_.Reset();
            this->res_.ConfigDecoder(this->GetDecoder());
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

        void ReceiveCallback(sharpen::Future<sharpen::Size> &future)
        {
            try
            {
                sharpen::Size size = future.Get();
                this->GetDecoder().Decode(this->readBuf_.Data(),size);
            }
            catch(const std::exception &)
            {
                this->FailReceive(std::current_exception());
                return;
            }

            bool complete = this->GetDecoder().IsCompleted();
            Waiter last;
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
        RpcClient(sharpen::NetStreamChannelPtr conn,const _Encoder &encoder = _Encoder{},const _Decoder &decoder = _Decoder{})
            :lock_()
            ,started_(false)
            ,pair_()
            ,reader_()
            ,conn_(conn)
            ,readBuf_(4096)
            ,waiters_()
            ,res_()
        {
            this->pair_.First() = std::move(encoder);
            this->pair_.Second() = std::move(decoder);
        }

        ~RpcClient() noexcept
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
            Waiter *pwaiter = nullptr;
            {
                std::unique_lock<Lock> lock(this->lock_);
                this->waiters_.push_back(std::move(waiter));
                pwaiter = &this->waiters_.back();
            }
            pwaiter->sender_.Get().SetCallback(std::bind(&Self::SendCallback,this,std::ref(future),std::placeholders::_1));
            this->conn_->WriteAsync(this->GetEncoder().Encode(package),0,pwaiter->sender_.Get());
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
}

#endif