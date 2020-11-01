#pragma once
#ifndef _SHARPEN_KQUEUE_HPP
#define _SHARPEN_KQUEUE_HPP

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_UNIX

#include <sys/event.h>

#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "FileTypeDef.hpp"

namespace sharpen
{
    class Kqueue:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        
        sharpen::FileHandle handle_;
    public:
        using Event = struct kevent;
    
        Kqueue();
        
        ~Kqueue() noexcept;
        
        sharpen::Uint32 Wait(Event *events,sharpen::Int32 maxEvent,int timeout);
        
        void Add(sharpen::FileHandle handle,sharpen::Int16 eventType,void *data);
        
        void Remove(sharpen::FileHandle handle);
        
        void Update(sharpen::FileHandle handle,sharpen::Int16 eventType,void *data);
    };
}

#endif
#endif
