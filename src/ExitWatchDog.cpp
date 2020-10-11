#include <sharpen/ExitWatchDog.hpp>

sharpen::ExitWatchDog::~ExitWatchDog() noexcept
{
    if(sharpen::LocalEngineContext)
    {
        this->ReleaseResource();
    }
}

void sharpen::ExitWatchDog::ReleaseResource() noexcept
{
#ifdef SHARPEN_HAS_FIBER
    sharpen::LocalEngineContext.reset(nullptr);
    ::ConvertFiberToThread();
#endif
}

thread_local sharpen::LocalWatchDog;
