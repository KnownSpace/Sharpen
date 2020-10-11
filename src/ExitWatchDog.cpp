#include <sharpen/ExitWatchDog.hpp>

sharpen::ExitWatchDog::~ExitWatchDog()
{
    if(sharpen::LocalEngineContext)
    {
    }
}

thread_local sharpen::LocalWatchDog;
