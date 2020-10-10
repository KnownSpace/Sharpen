#pragma once
#ifndef _SHARPEN_WINFIBERWATCHDOG_HPP
#define _SHARPEN_WINFIBERWATCHDOG_HPP

#include "SystemMacro.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"

#ifdef SHARPEN_IS_WIN
namespace sharpen
{
    class WinFiberWatchDog:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        
    public:
    };
    
    extern thread_local WinFiberWatchDog FiberWatchDog;
}
#endif

#endif
