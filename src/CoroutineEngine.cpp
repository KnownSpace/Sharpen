#include <sharpen/CoroutineEngine.hpp>

thread_local std::unique_ptr<sharpen::ExecuteContext> sharpen::LocalEngineContext(nullptr);

thread_local std::unique_ptr<sharpen::ExecuteContext> sharpen::LocalFromContext(nullptr);

sharpen::CoroutineEngine sharpen::CentralEngine;

void sharpen::CouroutineEngine::InitThisThread()
{
    sharpen::ExecuteContext::InternalEnableContextSwitch();
}

void sharpen::CentralEngineLoopEntry(void *lpNull)
{
    while(sharpen::CentralEngine.IsAlive())
    {
        if(sharpen::LocalSwitchCallback)
        {
            auto fn = std::move(sharpen::LocalSwitchCallback);
            fn();
        }
        std::unique_ptr<sharpen::ExecuteContext> ctx = std::move(sharpen::CentralEngine.WaitContext());
        ctx->Switch(sharpen::LocalEngineContext);
    }
}
