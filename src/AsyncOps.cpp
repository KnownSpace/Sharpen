#include <sharpen/AsyncOps.hpp>

void sharpen::YieldTimeslice()
{
    sharpen::InitThisThreadForCentralEngine();
    sharpen::ExecuteContextPtr *current = new sharpen::ExecuteContextPtr(std::move(sharpen::ExecuteContext::GetCurrentContext()));
    sharpen::LocalContextSwitchCallback = [current]()
    {
        std::unique_ptr<sharpen::ExecuteContextPtr> p(current);
        sharpen::ExecuteContextPtr ctx = *p;
        sharpen::CentralEngine.PushContext(std::move(ctx));
    };
    sharpen::LocalEngineContext->Switch();
}