#include <cassert>

#include <sharpen/CoroutineEngine.hpp>
#include <sharpen/ExitWatchDog.hpp>

thread_local std::unique_ptr<sharpen::ExecuteContext> sharpen::LocalEngineContext(nullptr);

thread_local std::unique_ptr<sharpen::IContextSwitchCallback> sharpen::LocalContextSwitchCallback;

sharpen::CoroutineEngine sharpen::CentralEngine;

void sharpen::InitThisThreadForCentralEngine()
{
    if(!sharpen::LocalEngineContext)
    {
        sharpen::ExecuteContext::InternalEnableContextSwitch();
        sharpen::LocalEngineContext = std::move(sharpen::ExecuteContext::MakeContext(std::bind(&sharpen::CentralEngineLoopEntry)));
        if(sharpen::LocalEngineContext == nullptr)
        {
            throw std::bad_alloc();
        }
        sharpen::LocalEngineContext->SetAutoRelease(true);
    }
}

void sharpen::CentralEngineLoopEntry()
{
    while(true)
    {
        if(sharpen::LocalContextSwitchCallback)
        {
            sharpen::LocalContextSwitchCallback->Run();
            sharpen::LocalContextSwitchCallback.release();
        }
        std::unique_ptr<sharpen::ExecuteContext> ctx = std::move(sharpen::CentralEngine.WaitContext());
        assert(ctx != nullptr);
        assert(sharpen::LocalEngineContext != nullptr);
        ctx->Switch(*sharpen::LocalEngineContext);
    }
}

sharpen::CoroutineEngine::CoroutineEngine()
    :contexts_()
{}

sharpen::CoroutineEngine::ContextPtr sharpen::CoroutineEngine::WaitContext() noexcept
{
    return std::move(this->contexts_.Pop());
}

void sharpen::CoroutineEngine::PushContext(sharpen::CoroutineEngine::ContextPtr context)
{
    this->contexts_.Push(std::move(context));
}

void sharpen::CoroutineEngine::InternalPushTask(std::function<void()> fn)
{
    std::function<void()> apply = [fn](){
        fn();
        sharpen::LocalEngineContext->Switch();
    };
    std::unique_ptr<sharpen::ExecuteContext> context = sharpen::ExecuteContext::MakeContext(std::move(apply));
    context->SetAutoRelease(true);
    this->PushContext(std::move(context));
}
