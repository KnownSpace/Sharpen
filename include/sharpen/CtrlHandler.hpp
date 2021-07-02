#pragma once
#ifndef _SHARPEN_CTRLHANDLER_HPP
#define _SHARPEN_CTRLHANDLER_HPP

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_WIN
#include <Windows.h>
#else
#include <signal.h>
#endif

#include <functional>

namespace sharpen
{
    enum class CtrlType
    {
        //Ctrl + C
        Interrupt,
        //Ctrl + \ (or close console on windows)
        Quite       
    };

    struct CtrlHelper
    {
        using Handler = std::function<void()>;

        static Handler onInterrupt_;

        static Handler onQuite_;

#ifdef SHARPEN_IS_WIN
        static BOOL CtrlHandler(DWORD ctrlType);
#else
        static void CtrlHandler(int signalType);
#endif

    };
    

    void RegisterCtrlHandler(sharpen::CtrlType type,sharpen::CtrlHelper::Handler handler);
}

#endif