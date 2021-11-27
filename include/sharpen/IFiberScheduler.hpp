#pragma once
#ifndef _SHARPEN_IFIBERSCHEDULER_HPP
#define _SHARPEN_IFIBERSCHEDULER_HPP

#include "Fiber.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"

#ifndef SHARPEN_FIBER_STACK_SIZE
#define SHARPEN_FIBER_STACK_SIZE 16*1024
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

        template<typename _Fn,typename ..._Args,typename _Check = decltype(std::bind(std::declval<_Fn>(),std::declval<_Args>()...)())>
        void Launch(_Fn &&fn,_Args &&...args)
        {
            this->LaunchSpecial(SHARPEN_FIBER_STACK_SIZE,std::forward<_Fn>(fn),std::forward<_Args>(args)...);
        }

        template<typename _Fn,typename ..._Args,typename _Check = decltype(std::bind(std::declval<_Fn>(),std::declval<_Args>()...)())>
        void LaunchSpecial(sharpen::Size stackSize,_Fn &&fn,_Args &&...args)
        {
            sharpen::FiberPtr fiber = sharpen::Fiber::MakeFiber(stackSize,std::forward<_Fn>(fn),std::forward<_Args>(args)...);
            fiber->SetScheduler(this);
            this->Schedule(std::move(fiber));
        }

        virtual bool IsProcesser() const = 0;

        virtual void SwitchToProcesserFiber() noexcept = 0;

        virtual void SetSwitchCallback(std::function<void()> fn) = 0;
    };
}

#endif