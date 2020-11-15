#pragma once
#ifndef _SHARPEN_FILECHANNEL_HPP
#define _SHARPEN_FILECHANNEL_HPP

#include "IChannel.hpp"
#include "Noncopyable.hpp"

namespace sharpen
{
    class FileHandle:public sharpen::IChannel,public sharpen::Noncopyable
    {
    private:
        
        sharpen::FileHandle handle_;
        
        sharpen::EventLoop *loop_;
    public:
    };
};

#endif
