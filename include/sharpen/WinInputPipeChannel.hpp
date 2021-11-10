#pragma once
#ifndef _SHARPEN_WININPUTPIPECHANNEL_HPP
#define _SHARPEN_WININPUTPIPECHANNEL_HPP

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_WIN

#include "IInputPipeChannel.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "IocpSelector.hpp"

#define SHARPEN_HAS_WININPUTPIPE

namespace sharpen
{
    class WinInputPipeChannel:public sharpen::IInputPipeChannel,public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Mybase = sharpen::IInputPipeChannel;

        static void InitOverlapped(OVERLAPPED &ol);

        void InitOverlappedStruct(sharpen::IocpOverlappedStruct &event);
    public:
        explicit WinInputPipeChannel(sharpen::FileHandle handle);
        
        virtual ~WinInputPipeChannel();

        virtual void ReadAsync(sharpen::Char *buf,sharpen::Size bufSize,sharpen::Future<sharpen::Size> &future) override;
        
        virtual void ReadAsync(sharpen::ByteBuffer &buf,sharpen::Size bufferOffset,sharpen::Future<sharpen::Size> &future) override;

        virtual void OnEvent(sharpen::IoEvent *event) override;
    };
}

#endif
#endif