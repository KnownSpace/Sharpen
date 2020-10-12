#include <sharpen/CoroutineEngine.hpp>

thread_local std::unique_ptr<sharpen::ExecuteContext> sharpen::LocalEngineContext(nullptr);

thread_local std::unique_ptr<sharpen::ExecuteContext> sharpen::LocalFromContext(nullptr);

void sharpen::CouroutineEngine::InitThisThread()
{
    sharpen::ExecuteContext::InternalEnableContextSwitch();
}
