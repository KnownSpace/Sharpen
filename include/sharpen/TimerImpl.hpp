#pragma once
#ifndef _SHARPEN_TIMERIMPL_HPP
#define _SHARPEN_TIMERIMPL_HPP

#include "LinuxTimer.hpp"
#include "SystemMacro.hpp"
#include "WinTimer.hpp"

namespace sharpen {
#ifdef SHARPEN_IS_WIN
    using TimerImpl = sharpen::WinTimer;
#else
    using TimerImpl = sharpen::LinuxTimer;
#endif
}   // namespace sharpen

#endif