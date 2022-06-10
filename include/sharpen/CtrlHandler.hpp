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
#include <vector>
#include <mutex>

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
        using Handlers = std::vector<Handler>;

        static Handlers onInterrupt_;

        static Handlers onQuite_;

#ifdef SHARPEN_IS_WIN
        static BOOL WINAPI CtrlHandler(DWORD ctrlType);
#else
        static void CtrlHandler(int signalType);
#endif

        static std::once_flag flag_;

    };
    

    void InitCtrlHandler();

    void RegisterCtrlHandler(sharpen::CtrlType type,sharpen::CtrlHelper::Handler handler);
}

#endif