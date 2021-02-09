#include <cassert>

#include <sharpen/CoroutineEngine.hpp>
#include <sharpen/ExitWatchDog.hpp>

thread_local sharpen::ExecuteContextPtr sharpen::LocalEngineContext(nullptr);

thread_local std::function<void()> sharpen::LocalContextSwitchCallback;

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
    }
}

void sharpen::CentralEngineLoopEntry()
{
    while(true)
    {
        if(sharpen::LocalContextSwitchCallback)
        {
            try
            {
                sharpen::LocalContextSwitchCallback();
            }
            catch(const std::exception& ignore)
            {
                assert(ignore.what());
                (void)ignore;
            }
            sharpen::LocalContextSwitchCallback = std::function<void()>();
        }
        sharpen::ExecuteContextPtr ctx = std::move(sharpen::CentralEngine.WaitContext());
        assert(ctx != nullptr);
        assert(sharpen::LocalEngineContext != nullptr);
        ctx->Switch();
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
    sharpen::ExecuteContextPtr context = sharpen::ExecuteContext::MakeContext([fn]() mutable {
        try
        {
            fn();
        }
        catch(const std::exception& ignore)
        {
            assert(ignore.what());
            (void)ignore;
        }
        sharpen::LocalEngineContext->Switch();
    });
    this->PushContext(std::move(context));
}
