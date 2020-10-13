#include <cassert>

#include <sharpen/CoroutineEngine.hpp>

thread_local std::unique_ptr<sharpen::ExecuteContext> sharpen::LocalEngineContext(nullptr);

thread_local std::function<void()> sharpen::LocalContextSwitchCallback;

sharpen::CoroutineEngine sharpen::CentralEngine;

void sharpen::InitThisThreadForCentralEngine()
{
    if(!sharpen::LocalEngineContext)
    {
        sharpen::ExexcuteContext::InternalEnableContextSwitch();
        sharpen::LocalEngineContext = std::move(sharpen::ExecuteContext::MakeContext(std::bind(&sharpen::CentralEngineLoopEntry)));
        assert(sharpen::LocalEngineContext != nullptr);
        sharpen::LocalEngineContext->SetAutoRelease(true);
    }
}

void sharpen::CentralEngineLoopEntry()
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
    return std::move(this->contexts.Pop());
}

void sharpen::CoroutineEngine::PushContext(sharpen::CoroutineEngine::ContextPtr context)
{
    this->context_.Push(std::move(context));
}

bool sharpen::CoroutineEngine::IsAlive() const
{
    return this->alive_;
}

void sharpen::CoroutineEngine::InternalPushTask(std::function<void()> &&fn)
{
    std::function<void> tmp = std::move(fn);
    std::function<void> apply = [tmp](){
        tmp();
        sharpen::LocalEngineContext->Switch();
    };
}
