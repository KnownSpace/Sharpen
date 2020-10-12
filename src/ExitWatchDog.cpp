#include <sharpen/ExitWatchDog.hpp>

sharpen::ExitWatchDog::~ExitWatchDog() noexcept
{
    if(sharpen::LocalEnableContextSwitch)
    {
        this->ReleaseResource();
    }
}

void sharpen::ExitWatchDog::ReleaseResource() noexcept
{
    sharpen::ExecuteContext::InternalDisableContextSwitch();
}

thread_local sharpen::LocalWatchDog;
