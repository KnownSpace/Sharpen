#pragma once
#ifndef _SHARPEN_INETSTREAMCHANNEL_HPP
#define _SHARPEN_INETSTREAMCHANNEL_HPP

#include "IChannel.hpp"
#include "Noncopyable.hpp"
#include "IAsyncReadable.hpp"
#include "IAsyncWritable.hpp"
#include "IFileChannel.hpp"
#include "IEndPoint.hpp"
#include "ITimer.hpp"
#include "Optional.hpp"

namespace sharpen
{

    class INetStreamChannel;

    using NetStreamChannelPtr = std::shared_ptr<sharpen::INetStreamChannel>;

    class INetStreamChannel:public sharpen::IChannel,public sharpen::IAsyncWritable,public sharpen::IAsyncReadable
    {
    private:
        using Self = sharpen::INetStreamChannel;

        inline static void CancelCallback(sharpen::Future<bool> &future,sharpen::INetStreamChannel *channel)
        {
            if(future.Get())
            {
                channel->Cancel();
            }
        }
    public:
        
        INetStreamChannel() noexcept = default;
        
        virtual ~INetStreamChannel() noexcept = default;
        
        INetStreamChannel(const Self &) = default;
        
        INetStreamChannel(Self &&) noexcept = default;

        virtual void SendFileAsync(sharpen::FileChannelPtr file,sharpen::Uint64 size,sharpen::Uint64 offset,sharpen::Future<void> &future) = 0;
        
        virtual void SendFileAsync(sharpen::FileChannelPtr file,sharpen::Future<void> &future) = 0;

        void SendFileAsync(sharpen::FileChannelPtr file,sharpen::Uint64 size,sharpen::Uint64 offset);

        void SendFileAsync(sharpen::FileChannelPtr file);

        virtual void AcceptAsync(sharpen::Future<sharpen::NetStreamChannelPtr> &future) = 0;

        sharpen::NetStreamChannelPtr AcceptAsync();

        virtual void ConnectAsync(const sharpen::IEndPoint &endpoint,sharpen::Future<void> &future) = 0;

        void ConnectAsync(const sharpen::IEndPoint &endpoint);

        void Bind(const sharpen::IEndPoint &endpoint);

        virtual void Listen(sharpen::Uint16 queueLength);

        void GetLocalEndPoint(sharpen::IEndPoint &endPoint) const;

        void GetRemoteEndPoint(sharpen::IEndPoint &endPoint) const;

        void SetKeepAlive(bool val);

        inline void DisableKeepAlive()
        {
            this->SetKeepAlive(false);
        }

        inline void EnableKeepAlive()
        {
            this->SetKeepAlive(true);
        }

        void SetReuseAddress(bool val);

        int GetErrorCode() const noexcept;

        virtual void PollReadAsync(sharpen::Future<void> &future) = 0;

        virtual void PollWriteAsync(sharpen::Future<void> &future) = 0;

        void PollReadAsync();

        void PollWriteAsync();

        virtual void Cancel() noexcept = 0;

        template<typename _Rep,typename _Period>
        inline sharpen::Optional<sharpen::Size> ReadWithTimeout(sharpen::TimerPtr timer,const std::chrono::duration<_Rep,_Period> &duration,char *buf,sharpen::Size size)
        {
            sharpen::AwaitableFuture<bool> timeout;
            sharpen::AwaitableFuture<sharpen::Size> future;
            using CancelFn = void(*)(sharpen::Future<bool>&,sharpen::INetStreamChannel*);
            timeout.SetCallback(std::bind(static_cast<CancelFn>(&Self::CancelCallback),std::placeholders::_1,this));
            this->ReadAsync(buf,size,future);
            timer->WaitAsync(timeout,duration);
            future.WaitAsync();
            //timeout
            if(future.IsError() && timeout.IsCompleted())
            {
                return sharpen::EmptyOpt;
            }
            //cancel timer
            if(timeout.IsPending())
            {
                timer->Cancel();
                timeout.WaitAsync();
            }
            return future.Get();
        }

        template<typename _Rep,typename _Period>
        inline sharpen::Optional<sharpen::Size> ReadWithTimeout(sharpen::TimerPtr timer,const std::chrono::duration<_Rep,_Period> &duration,sharpen::ByteBuffer &buf,sharpen::Size offset)
        {
            assert(buf.GetSize() >= offset);
            return this->ReadWithTimeout(std::move(timer),duration,buf.Data() + offset,buf.GetSize() - offset);
        }

        template<typename _Rep,typename _Period>
        inline sharpen::Optional<sharpen::Size> ReadWithTimeout(sharpen::TimerPtr timer,const std::chrono::duration<_Rep,_Period> &duration,sharpen::ByteBuffer &buf)
        {
            return this->ReadWithTimeout(std::move(timer),duration,buf,0);
        }

        template<typename _Rep,typename _Period>
        inline bool ConnectWithTimeout(sharpen::TimerPtr timer,const std::chrono::duration<_Rep,_Period> &duration,const sharpen::IEndPoint &ep)
        {
            sharpen::AwaitableFuture<bool> timeout;
            sharpen::AwaitableFuture<void> future;
            using CancelFn = void(*)(sharpen::Future<bool>&,sharpen::INetStreamChannel*);
            timeout.SetCallback(std::bind(static_cast<CancelFn>(&Self::CancelCallback),std::placeholders::_1,this));
            this->ConnectAsync(ep,future);
            timer->WaitAsync(timeout,duration);
            future.WaitAsync();
            //timeout
            if(future.IsError() && timeout.IsCompleted())
            {
                return false;
            }
            //cancel timer
            if(timeout.IsPending())
            {
                timer->Cancel();
                timeout.WaitAsync();
            }
            future.Get();
            return true;
        }
    };

    sharpen::NetStreamChannelPtr MakeTcpStreamChannel(sharpen::AddressFamily af);

    void StartupNetSupport();

    void CleanupNetSupport();
}

#endif
