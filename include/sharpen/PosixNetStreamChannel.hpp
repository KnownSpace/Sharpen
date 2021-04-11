#pragma once
#ifndef _SHARPEN_POSIXNETSTREAMCHANNEL_HPP
#define _SHARPEN_POSIXNETSTREAMCHANNEL_HPP

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_NIX

#define SHARPEN_HAS_POSIXSOCKET

#include "INetStreamChannel.hpp"

namespace sharpen
{
    class PosixNetStreamChannel:public sharpen::INetStreamChannel,public sharpen::Noncopyable
    {
    private:

    protected:

        virtual void DoClose() noexcept override;

    public:

        virtual void WriteAsync(const sharpen::Char *buf,sharpen::Size bufSize,sharpen::Future<sharpen::Size> &future) override;
        
        virtual void WriteAsync(const sharpen::ByteBuffer &buf,sharpen::Future<sharpen::Size> &future) override;

        virtual void ReadAsync(sharpen::Char *buf,sharpen::Size bufSize,sharpen::Future<sharpen::Size> &future) override;
        
        virtual void ReadAsync(sharpen::ByteBuffer &buf,sharpen::Future<sharpen::Size> &future) override;

        virtual void OnEvent(sharpen::IoEvent *event) override;
    };
};

#endif
#endif
