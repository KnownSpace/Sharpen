#include <sharpen/CtrlHandler.hpp>

sharpen::CtrlHelper::Handlers sharpen::CtrlHelper::onInterrupt_;

sharpen::CtrlHelper::Handlers sharpen::CtrlHelper::onQuite_;

std::once_flag sharpen::CtrlHelper::flag_;

#ifdef SHARPEN_IS_WIN

BOOL sharpen::CtrlHelper::CtrlHandler(DWORD ctrlType)
{
    if (ctrlType == CTRL_C_EVENT)
    {
        for (auto begin = sharpen::CtrlHelper::onInterrupt_.begin();begin != sharpen::CtrlHelper::onInterrupt_.end();++begin)
        {
            (*begin)();
        }
    }
    else if (ctrlType == CTRL_CLOSE_EVENT)
    {
        for (auto begin = sharpen::CtrlHelper::onInterrupt_.begin();begin != sharpen::CtrlHelper::onInterrupt_.end();++begin)
        {
            (*begin)();
        } 
    }
    return FALSE;
}

#else

void sharpen::CtrlHelper::CtrlHandler(int signalType)
{
    if (signalType == SIGINT)
    {
        for (auto begin = sharpen::CtrlHelper::onInterrupt_.begin();begin != sharpen::CtrlHelper::onInterrupt_.end();++begin)
        {
            (*begin)();
        }
    }
    else if(signalType == SIGQUIT)
    {
        for (auto begin = sharpen::CtrlHelper::onInterrupt_.begin();begin != sharpen::CtrlHelper::onInterrupt_.end();++begin)
        {
            (*begin)();
        } 
    }
}

#endif

void sharpen::RegisterCtrlHandler(sharpen::CtrlType type,sharpen::CtrlHelper::Handler handler)
{
    switch (type)
    {
    case sharpen::CtrlType::Interrupt:
        sharpen::CtrlHelper::onInterrupt_.push_back(std::move(handler));
        break;
    case sharpen::CtrlType::Quite:
        sharpen::CtrlHelper::onQuite_.push_back(std::move(handler));
        break;
    default:
        break;
    }
    std::call_once(sharpen::CtrlHelper::flag_,[]()
    {
#ifdef SHARPEN_IS_WIN
        using FnPtr = BOOL(*)(DWORD);
        ::SetConsoleCtrlHandler(reinterpret_cast<FnPtr>(&sharpen::CtrlHelper::CtrlHandler),TRUE);
#else
        using FnPtr = void(*)(int);
        ::signal(SIGINT,reinterpret_cast<FnPtr>(&sharpen::CtrlHelper::CtrlHandler));
        ::signal(SIGQUIT,reinterpret_cast<FnPtr>(&sharpen::CtrlHelper::CtrlHandler));
#endif
    });
}