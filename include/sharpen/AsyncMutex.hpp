#pragma once
#ifndef _SHARPEN_ASYNCMUTEX_HPP
#define _SHARPEN_ASYNCMUTEX_HPP

#include <list>
#include <functional>

#include "Future.hpp"

namespace sharpen
{
    class AsyncMutex:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
        
    private:
        using Function = std::function<void()>;
        using List = std::list<Function>;

        bool locked_;
        List waiters_;
        sharpen::SpinLock lock_;
    public:
        AsyncMutex();

        void Lock(Function &&callback);

        sharpen::SharedFuturePtr<void> LockAsync();

        void Unlock();

        ~AsyncMutex() = default;
    };
    
}

#endif