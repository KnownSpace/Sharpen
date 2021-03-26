#pragma once
#ifndef _SHARPEN_POSIXSOCKETCHANNEL_HPP
#define _SHARPEN_POSIXSOCKETCHANNEL_HPP

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_NIX

#define SHARPEN_HAS_POSIXSOCKET

#include "ISocketChannel.hpp"

namespace sharpen
{
    class PosixSocketChannel:public sharpen::ISocketChannel,public sharpen::Noncopyable
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
