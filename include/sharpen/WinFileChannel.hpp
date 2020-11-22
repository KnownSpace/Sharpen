#pragma once
#ifndef _SHARPEN_WINFILECHANNEL_HPP
#define _SHARPEN_WINFILECHANNEL_HPP

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_WIN

#define SHARPEN_USE_WINFILE

#include "IFileChannel.hpp"

namespace sharpen
{
    class WinFileChannel:public sharpen::IFileChannel,public sharpen:Noncopyable
    {
    private:
        sharpen::EventLoop *loop_;

        sharpen::FileHandle handle_;
    public:
    };
    
    using NativeFileChannel = sharpen::WinFileChannel;
}

#endif
#endif
