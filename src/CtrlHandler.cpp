#include <sharpen/CtrlHandler.hpp>

sharpen::CtrlHelper::Handler sharpen::CtrlHelper::onInterrupt_;

sharpen::CtrlHelper::Handler sharpen::CtrlHelper::onQuite_;

#ifdef SHARPEN_IS_WIN

BOOL sharpen::CtrlHelper::CtrlHandler(DWORD ctrlType)
{
    if (ctrlType == CTRL_C_EVENT)
    {
        if (sharpen::CtrlHelper::onInterrupt_)
        {
            sharpen::CtrlHelper::onInterrupt_();
            return TRUE;
        }
    }
    else if (ctrlType == CTRL_CLOSE_EVENT)
    {
        if (sharpen::CtrlHelper::onQuite_)
        {
            sharpen::CtrlHelper::onQuite_();
            return TRUE;
        }
    }
    return FALSE;
}

#else

void sharpen::CtrlHelper::CtrlHandler(int signalType)
{
    if (signalType == SIGINT)
    {
        if (sharpen::CtrlHelper::onInterrupt_)
        {
            sharpen::CtrlHelper::onInterrupt_();
        }
    }
    else if(signalType == SIGQUIT)
    {
        if (sharpen::CtrlHelper::onQuite_)
        {
            sharpen::CtrlHelper::onQuite_();
        }
    }
}

#endif

void sharpen::RegisterCtrlHandler(sharpen::CtrlType type,sharpen::CtrlHelper::Handler handler)
{
#ifdef SHARPEN_IS_WIN
    using SignalType = DWORD;
#else
    using SignalType = int;
#endif

#ifdef SHARPEN_IS_NIX
    SignalType sign;
#endif

    switch (type)
    {
    case sharpen::CtrlType::Interrupt:
        sharpen::CtrlHelper::onInterrupt_ = std::move(handler);
#ifdef SHARPEN_IS_NIX
        sign = SIGINT;
#endif
        break;
    case sharpen::CtrlType::Quite:
        sharpen::CtrlHelper::onQuite_ = std::move(handler);
#ifdef SHARPEN_IS_NIX
        sign = SIGQUIT;
#endif
        break;
    default:
        break;
    }
#ifdef SHARPEN_IS_WIN
    using FnPtr = BOOL(*)(DWORD);
    ::SetConsoleCtrlHandler(reinterpret_cast<FnPtr>(&sharpen::CtrlHelper::CtrlHandler),TRUE);
#else
    using FnPtr = void(*)(int);
    ::signal(sign,reinterpret_cast<FnPtr>(&sharpen::CtrlHelper::CtrlHandler));
#endif
}