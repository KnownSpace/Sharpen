#pragma once
#ifndef _SHARPEN_IOCOMPLETIONPORT_HPP

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_WIN

#define SHARPEN_HAS_IOCP

#include <Windows.h>

#include "FileTypeDef.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"

namespace sharpen
{
    class IoCompletionPort:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        sharpen::FileHandle handle_;
    public:
        using Event = OVERLAPPED_ENTRY;
        
        using Overlapped = OVERLAPPED;
        
        IoCompletionPort();
        
        ~IoCompletionPort() noexcept;
        
        sharpen::Uint32 Wait(Event *events,sharpen::Uint32 maxEvents,sharpen::Uint32 timeout);
        
        void Bind(sharpen::FileHandle handle);
        
        void Post(Overlapped *overlapped,sharpen::Uint32 bytesTransferred,void *completionKey);
        
        void Notify();
    };
}

#endif
#endif
