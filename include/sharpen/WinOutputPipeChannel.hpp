#pragma once
#ifndef _SHARPEN_WINOUTPUTPIPECHANNEL_HPP
#define _SHARPEN_WINOUTPUTPIPECHANNEL_HPP

#include "IOutputPipeChannel.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "IocpSelector.hpp"
#ifdef SHARPEN_IS_WIN
#define SHARPEN_HAS_WINOUTPUTPIPE

namespace sharpen
{
    class WinOutputPipeChannel:public sharpen::IOutputPipeChannel,public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Mybase = sharpen::IOutputPipeChannel;

        static void InitOverlapped(OVERLAPPED &ol);

        void InitOverlappedStruct(sharpen::IocpOverlappedStruct &olStruct);
    
    public:
        explicit WinOutputPipeChannel(sharpen::FileHandle handle);

        virtual ~WinOutputPipeChannel() = default;

        virtual void WriteAsync(const sharpen::Char *buf,sharpen::Size bufSize,sharpen::Future<sharpen::Size> &future) override;
        
        virtual void WriteAsync(const sharpen::ByteBuffer &buf,sharpen::Size bufferOffset,sharpen::Future<sharpen::Size> &future) override;

        virtual void OnEvent(sharpen::IoEvent *event) override;
    };
}

#endif
#endif