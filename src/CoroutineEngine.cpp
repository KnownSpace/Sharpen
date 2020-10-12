#include <sharpen/CoroutineEngine.hpp>

thread_local std::unique_ptr<sharpen::ExecuteContext> sharpen::LocalEngineContext(nullptr);

thread_local std::unique_ptr<sharpen::ExecuteContext> sharpen::LocalFromContext(nullptr);
