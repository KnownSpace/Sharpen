#include <sharpen/ThreadGuard.hpp>

sharpen::ThreadGuard::~ThreadGuard() noexcept
{
    if(sharpen::LocalEnableContextSwitch)
    {
        this->ReleaseResource();
    }
}

void sharpen::ThreadGuard::ReleaseResource() noexcept
{
    sharpen::ExecuteContext::InternalDisableContextSwitch();
}