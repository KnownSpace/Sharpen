#include <cassert>

#include <sharpen/CoroutineEngine.hpp>

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
    while(sharpen::CentralEngine.IsAlive())
    {
        if(sharpen::LocalContextSwitchCallback)
        {
            sharpen::LocalContextSwitchCallback->Run();
            sharpen::LocalContextSwitchCallback.release();
        }
        std::unique_ptr<sharpen::ExecuteContext> ctx = std::move(sharpen::CentralEngine.WaitContext());
        assert(sharpen::LocalEngineContext != nullptr);
        ctx->Switch(*sharpen::LocalEngineContext);
    }
}

sharpen::CoroutineEngine::CoroutineEngine()
    :contexts_()
    ,alive_(true)
{}

sharpen::CoroutineEngine::~CoroutineEngine() noexcept
{
    this->alive_ = false;
}

sharpen::CoroutineEngine::ContextPtr sharpen::CoroutineEngine::WaitContext()
{
    return std::move(this->contexts_.Pop());
}

void sharpen::CoroutineEngine::PushContext(sharpen::CoroutineEngine::ContextPtr context) noexcept
{
    this->contexts_.Push(std::move(context));
}

bool sharpen::CoroutineEngine::IsAlive() const
{
    return this->alive_;
}

void sharpen::CoroutineEngine::InternalPushTask(std::function<void()> &&fn)
{
    std::function<void()> tmp (std::move(fn));
    std::function<void()> apply = [tmp]()mutable {
        tmp();
        sharpen::LocalEngineContext->Switch();
    };
    std::unique_ptr<sharpen::ExecuteContext> context = sharpen::ExecuteContext::MakeContext(std::move(apply));
    context->SetAutoRelease(true);
    this->PushContext(std::move(context));
}
