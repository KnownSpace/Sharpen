#include <sharpen/SignalLock.hpp>

#ifndef SHARPEN_IS_WIN
thread_local sigset_t sharpen::SignalLock::oldSet_;
#endif

#ifndef SHARPEN_IS_WIN
void sharpen::SignalLock::GetBlockableSet(sigset_t &set) noexcept
{
    ::sigfillset(&set);
    ::sigdelset(&set,SIGKILL);
    ::sigdelset(&set,SIGSTOP);
    ::sigdelset(&set,SIGCONT);
}
#endif

void sharpen::SignalLock::Lock() noexcept
{
#ifndef SHARPEN_IS_WIN
    sigset_t newSet;
    Self::GetBlockableSet(newSet);
    if(::pthread_sigmask(SIG_SETMASK,&newSet,&Self::oldSet_) != 0)
    {
        std::terminate();
    }
#endif
    return this->lock_.lock();
}

void sharpen::SignalLock::Unlock() noexcept
{
    this->lock_.unlock();
#ifndef SHARPEN_IS_WIN
    if(::pthread_sigmask(SIG_SETMASK,&Self::oldSet_,nullptr) == -1)
    {
        std::terminate();
    }
#endif
}