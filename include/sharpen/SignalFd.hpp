#pragma once
#ifndef _SHARPEN_SIGNALFD_HPP
#define _SHARPEN_SIGNALFD_HPP

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_LINUX

#define SHARPEN_HAS_SIGNALFD

#include <sys/signalfd.h>
#include <unistd.h>

#include "FileTypeDef.hpp"

namespace sharpen
{
    extern sharpen::FileHandle OpenSignalFd(const sigset_t &sigs) noexcept;

    extern ssize_t ReadSignalFd(sharpen::FileHandle fd,signalfd_siginfo &sigInfo) noexcept;
}

#endif
#endif