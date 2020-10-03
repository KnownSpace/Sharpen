#pragma once
#ifndef _SHARPEN_ASYNCSEMAPHORE_HPP
#define _SHARPEN_ASYNCSEMAPHORE_HPP

#include <list>
#include <functional>

#include "Future.hpp"
#include "TypeDef.hpp"

namespace sharpen
{

    class AsyncSemaphore:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Function = std::function<void()>;
        using List = std::list<Function>;

        List waiters_;
        sharpen::SpinLock lock_;
        sharpen::Uint32 counter_;

        bool NeedWait() const;
    public:
        AsyncSemaphore(sharpen::Uint32 count);

        void Lock(Function &&callback);

        sharpen::SharedFuturePtr<void> LockAsync();

        void Unlock();

        ~AsyncSemaphore() = default;
    };
    
}

#endif