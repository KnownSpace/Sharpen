#pragma once
#ifndef _SHARPEN_IOCOMPLETIONPORT_HPP

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_WIN

#define SHARPEN_HAS_IOCP

#include <Windows.h>

#include "FileTypeDef.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include <cstdint>
#include <cstddef>

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
        
        std::uint32_t Wait(Event *events,std::uint32_t maxEvents,std::uint32_t timeout);
        
        void Bind(sharpen::FileHandle handle);
        
        void Post(Overlapped *overlapped,std::uint32_t bytesTransferred,void *completionKey);
        
        void Notify();
    };
}

#endif
#endif
