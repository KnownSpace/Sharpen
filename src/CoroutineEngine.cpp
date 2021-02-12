#include <cassert>

#include <sharpen/CoroutineEngine.hpp>
#include <sharpen/ThreadGuard.hpp>

thread_local sharpen::ExecuteContextPtr sharpen::LocalSchedulerContext(nullptr);

thread_local std::function<void()> sharpen::LocalContextSwitchCallback;

sharpen::CoroutineEngine sharpen::CentralEngine;

//thread_local sharpen::ThreadGuard sharpen::LocalThreadGuard;

void sharpen::InitThisThreadForCentralEngine()
{
    if(!sharpen::LocalSchedulerContext)
    {
        sharpen::ExecuteContext::InternalEnableContextSwitch();
        sharpen::LocalSchedulerContext = std::move(sharpen::ExecuteContext::MakeContext(std::bind(&sharpen::ScheduleLoop)));
        if(sharpen::LocalSchedulerContext == nullptr)
        {
            throw std::bad_alloc();
        }
    }
}

void sharpen::ScheduleLoop()
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
        if (!ctx)
        {
            continue;
        }
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
        sharpen::LocalSchedulerContext->Switch();
    });
    this->PushContext(std::move(context));
}
