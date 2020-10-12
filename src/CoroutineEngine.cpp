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
        if(sharpen::LocalFromContext)
        {
            sharpen::CentralEngine.PushContext(std::move(sharpen::LocalFromContext));
        }
        std::unique_ptr<sharpen::ExecuteContext> ctx = std::move(sharpen::CentralEngine.WaitContext());
        ctx->Switch(sharpen::LocalEngineContext);
    }
}
