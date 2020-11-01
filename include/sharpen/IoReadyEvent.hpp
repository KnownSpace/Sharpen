#pragma once
#ifndef _SHARPEN_IOREADYEVENT
#define _SHARPEN_IOREADYEVENT

#include "IoEvent.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "FileTypeDef.hpp"

namespace sharpen
{
    class IoReadyEvent:public sharpen::Noncopyable,public sharpen::Nonmovable,public sharpen::IoEvent
    { 
    private:
        using Self = sharpen::IoReadyEvent;
        using Base = sharpen::IoEvent;
        
    protected:
        
        //file handle
        sharpen::FileHandle handle_;
    public:
        IoReadyEvent(Base::EventType type,sharpen::FileHandle handle)
            :Base(type)
            ,handle_(handle)
        {}
        
        ~IoReadyEvent() = default;
        
    };
}

#endif
