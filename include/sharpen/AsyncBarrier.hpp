#pragma once
#ifndef _SHARPEN_ASYNCBARRIER_HPP
#define _SHARPEN_ASYNCBARRIER_HPP

#include <list>
#include <functional>

#include "TypeDef.hpp"
#include "Future.hpp"

namespace sharpen
{
    class AsyncBarrier:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Function = std::function<void()>;
        using List = std::list<Function>;

        sharpen::Uint32 counter_;
        List waiters_;
        sharpen::Uint32 beginCounter_;
        sharpen::SpinLock lock_;
    public:
        AsyncBarrier(sharpen::Uint32 count);

        void Wait(Function &&callback);

        sharpen::SharedFuturePtr<void> WaitAsync();

        void Notice();

        ~AsyncBarrier() = default;
    };
    
}

#endif