#pragma once
#ifndef _SHARPEN_AWAITER_HPP
#define _SHARPEN_AWAITER_HPP

#include <type_traits>
#include <cassert>
#include <vector>

#include "SpinLock.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "IFiberScheduler.hpp"

namespace sharpen
{
    class Awaiter:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Self = sharpen::Awaiter;
        using Lock = sharpen::SpinLock;

        sharpen::IFiberScheduler *scheduler_;
        sharpen::FiberPtr fiber_;
        Lock lock_;
    public:
        explicit Awaiter(sharpen::IFiberScheduler *scheduler);

        ~Awaiter() noexcept = default;

        void Notify();

        void Wait(sharpen::FiberPtr fiber);
    };
    
}

#endif
