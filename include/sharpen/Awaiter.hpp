#pragma once
#ifndef _SHARPEN_AWAITER_HPP
#define _SHARPEN_AWAITER_HPP

#include <type_traits>
#include <cassert>
#include <vector>

#include "Future.hpp"
#include "FiberScheduler.hpp"

namespace sharpen
{
    class Awaiter:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Self = sharpen::Awaiter;
        using Lock = sharpen::SpinLock;

        sharpen::FiberPtr fiber_;
        Lock lock_;
    public:
        Awaiter();

        ~Awaiter() noexcept = default;

        void Notify();

        void Wait(sharpen::FiberPtr fiber);
    };
    
}

#endif
