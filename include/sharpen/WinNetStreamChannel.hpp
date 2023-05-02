#pragma once
#ifndef _SHARPEN_WINNETSTREAMCHANNEL_HPP
#define _SHARPEN_WINNETSTREAMCHANNEL_HPP

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_WIN
#define SHARPEN_HAS_WINSOCKET

#include "INetStreamChannel.hpp"
#include "WSAOverlappedStruct.hpp"

namespace sharpen {
    class WinNetStreamChannel
        : public sharpen::INetStreamChannel
        , public sharpen::Noncopyable {
    private:
        using Mybase = sharpen::INetStreamChannel;

        static void Closer(sharpen::FileHandle handle) noexcept;

        static void InitOverlapped(OVERLAPPED &ol);

        void InitOverlappedStruct(sharpen::WSAOverlappedStruct &olStruct);

        void HandleReadAndWrite(WSAOverlappedStruct &olStruct);

        void HandleAccept(WSAOverlappedStruct &olStruct);

        void HandleSendFile(WSAOverlappedStruct &olStruct);

        void HandleConnect(WSAOverlappedStruct &olStruct);

        void HandlePoll(WSAOverlappedStruct &olStruct);

        void RequestRead(char *buf, std::size_t bufSize, sharpen::Future<std::size_t> *future);

        void RequestWrite(const char *buf,
                          std::size_t bufSize,
                          sharpen::Future<std::size_t> *future);

        void RequestSendFile(sharpen::FileChannelPtr file,
                             std::uint64_t size,
                             std::uint64_t offset,
                             sharpen::Future<std::size_t> *future);

        void RequestConnect(const sharpen::IEndPoint *endpoint, sharpen::Future<void> *future);

        void RequestAccept(sharpen::Future<sharpen::NetStreamChannelPtr> *future);

        void RequestPollRead(sharpen::Future<void> *future);

        void RequestPollWrite(sharpen::Future<void> *future);

        void RequestCancel() noexcept;

        int af_;

    public:
        WinNetStreamChannel(sharpen::FileHandle handle, int af);

        virtual ~WinNetStreamChannel() noexcept = default;

        virtual void WriteAsync(const char *buf,
                                std::size_t bufSize,
                                sharpen::Future<std::size_t> &future) override;

        virtual void WriteAsync(const sharpen::ByteBuffer &buf,
                                std::size_t bufferOffset,
                                sharpen::Future<std::size_t> &future) override;

        virtual void ReadAsync(char *buf,
                               std::size_t bufSize,
                               sharpen::Future<std::size_t> &future) override;

        virtual void ReadAsync(sharpen::ByteBuffer &buf,
                               std::size_t bufferOffset,
                               sharpen::Future<std::size_t> &future) override;

        virtual void OnEvent(sharpen::IoEvent *event) override;

        virtual void SendFileAsync(sharpen::FileChannelPtr file,
                                   std::uint64_t size,
                                   std::uint64_t offset,
                                   sharpen::Future<std::size_t> &future) override;

        virtual void SendFileAsync(sharpen::FileChannelPtr file,
                                   sharpen::Future<std::size_t> &future) override;

        virtual void AcceptAsync(sharpen::Future<sharpen::NetStreamChannelPtr> &future) override;

        virtual void ConnectAsync(const sharpen::IEndPoint &endpoint,
                                  sharpen::Future<void> &future) override;

        virtual void PollReadAsync(sharpen::Future<void> &future) override;

        virtual void PollWriteAsync(sharpen::Future<void> &future) override;

        virtual void Cancel() noexcept override;
    };
};   // namespace sharpen

#endif
#endif
