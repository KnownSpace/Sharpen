#pragma once
#ifndef _SHARPEN_TIMERIMPL_HPP
#define _SHARPEN_TIMERIMPL_HPP

#include "SystemMacro.hpp"
#include "WinTimer.hpp"
#include "LinuxTimer.hpp"

namespace sharpen
{
#ifdef SHARPEN_IS_WIN
    using TimerImpl = sharpen::WinTimer;
#else
    using TimerImpl = sharpen::LinuxTimer;
#endif
}

#endif