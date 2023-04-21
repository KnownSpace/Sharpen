#include <sharpen/SignalFd.hpp>

#ifdef SHARPEN_HAS_SIGNALFD

sharpen::FileHandle sharpen::OpenSignalFd(const sigset_t &sigs) noexcept
{
    return ::signalfd(-1, &sigs, SFD_CLOEXEC | SFD_NONBLOCK);
}

ssize_t sharpen::ReadSignalFd(sharpen::FileHandle fd, signalfd_siginfo &sigBuf) noexcept
{
    return ::read(fd, &sigBuf, sizeof(sigBuf));
}

#endif