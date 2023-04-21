#pragma once
#ifndef _SHARPEN_WINOUTPUTPIPECHANNEL_HPP
#define _SHARPEN_WINOUTPUTPIPECHANNEL_HPP

#include "IOutputPipeChannel.hpp"
#include "IocpSelector.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#ifdef SHARPEN_IS_WIN
#define SHARPEN_HAS_WINOUTPUTPIPE

namespace sharpen
{
    class WinOutputPipeChannel
        : public sharpen::IOutputPipeChannel
        , public sharpen::Noncopyable
        , public sharpen::Nonmovable
    {
    private:
        using Mybase = sharpen::IOutputPipeChannel;
        using Self = sharpen::WinOutputPipeChannel;

        static void InitOverlapped(OVERLAPPED &ol);

        void InitOverlappedStruct(sharpen::IocpOverlappedStruct &olStruct);

        void RequestWrite(const char *buf,
                          std::size_t bufSize,
                          sharpen::Future<std::size_t> *future);

    public:
        explicit WinOutputPipeChannel(sharpen::FileHandle handle);

        virtual ~WinOutputPipeChannel();

        virtual void WriteAsync(const char *buf,
                                std::size_t bufSize,
                                sharpen::Future<std::size_t> &future) override;

        virtual void WriteAsync(const sharpen::ByteBuffer &buf,
                                std::size_t bufferOffset,
                                sharpen::Future<std::size_t> &future) override;

        virtual void OnEvent(sharpen::IoEvent *event) override;
    };
}   // namespace sharpen

#endif
#endif