#pragma once
#ifndef _SHARPEN_FILECHANNEL_HPP
#define _SHARPEN_FILECHANNEL_HPP

#include "WinFileChannel.hpp"
#include "PosixFileChannel.hpp"

namespace sharpen
{
    class FileChannel:public sharpen::IFileChannel,public sharpen::Noncopyable
    {
    private:
        using MyImpl = sharpen::NativeFileChannel;
        using MyImplPtr = std::unique_ptr<MyImpl>;
        
        MyImplPtr impl_;
    public:
    };
}

#endif
