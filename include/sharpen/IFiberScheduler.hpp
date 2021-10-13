#pragma once
#ifndef _SHARPEN_IFIBERSCHEDULER_HPP
#define _SHARPEN_IFIBERSCHEDULER_HPP

#include "Fiber.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"

#ifndef SHARPEN_FIBER_STACK_SIZE
#define SHARPEN_FIBER_STACK_SIZE 64*1024
#endif

namespace sharpen
{
    class IFiberScheduler:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:

    public:
        IFiberScheduler() noexcept = default;

        virtual ~IFiberScheduler() noexcept = default;

        virtual void Schedule(sharpen::FiberPtr &&fiber) = 0;

        template<typename _Fn,typename ..._Args>
        void Launch(_Fn &&fn,_Args &&...args)
        {
            sharpen::FiberPtr fiber = sharpen::Fiber::MakeFiber(SHARPEN_FIBER_STACK_SIZE,std::forward<_Fn>(fn),std::forward<_Args>(args)...);
            this->Schedule(std::move(fiber));
        }

        virtual bool IsProcesser() const = 0;

        virtual void SwitchToProcesserFiber() noexcept = 0;

        virtual void SetSwitchCallback(std::function<void()> fn) = 0;
    };
}

#endif