#pragma once
#ifndef _SHARPEN_POSIXFILECHANNEL_HPP
#define _SHARPEN_POSIXFILECHANNEL_HPP

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_NIX

#define SHARPEN_USE_POSIXFILE

#include "IFileChannel.hpp"

namespace sharpen
{
    class PosixFileChannel:public sharpen::IFileChannel,public sharpen::Noncopyable
    {
    private:
        sharpen::EventLoop *loop_;
        
        sharpen::FileHandle handle_;
    public:
    };
    
    using NativeFileChannel = sharpen::PosixFileChannel;
}

#endif
#endif
