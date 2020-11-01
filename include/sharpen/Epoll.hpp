#pragma once
#ifndef _SHARPEN_EPOLL_HPP
#define _SHARPEN_EPOLL_HPP

#include "SystemMacro.hpp"

//epoll is only support by linux
#ifdef SHARPEN_IS_LINUX

#define SHARPEN_HAS_EPOLL

#include <sys/epoll.h>

#include "FileTypeDef"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"

namespace sharpen
{
    class Epoll:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        sharpen::FileHandle handle_;
    public:
        using Event = epoll_event;
    
        Epoll();
        
        ~Epoll() noexcept;
    
        void Wait(Event *events,sharpen::Int32 maxEvents,sharpen::Int32 timeout);
        
        void Add(sharpen::FileHandle handle,Event *event);
        
        void Remove(sharpen::FileHandle handle);
        
        void Update(sharpen::FileHandle handle,Event *event);
    };
}

#endif
#endif
