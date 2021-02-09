#pragma once
#ifndef _SHARPEN_AWAITER_HPP
#define _SHARPEN_AWAITER_HPP

#include <type_traits>
#include <cassert>
#include <vector>

#include "Future.hpp"
#include "CoroutineEngine.hpp"

namespace sharpen
{
    class Awaiter:public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::Awaiter;
        using Lock = sharpen::SpinLock;
        using LockPtr = std::unique_ptr<Lock>;

        sharpen::ExecuteContextPtr waiter_;
    public:
        Awaiter();

        Awaiter(Self &&other) noexcept;

        ~Awaiter() noexcept = default;

        Self &operator=(Self &&other) noexcept;

        void Notify();

        void Wait(sharpen::ExecuteContextPtr context);
    };
    
}

#endif
