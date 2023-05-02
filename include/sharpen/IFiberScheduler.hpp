#pragma once
#ifndef _SHARPEN_IFIBERSCHEDULER_HPP
#define _SHARPEN_IFIBERSCHEDULER_HPP

#include "Fiber.hpp"
#include "Future.hpp"
#include "FutureCompletor.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "TypeTraits.hpp"

#ifndef SHARPEN_FIBER_STACK_SIZE
// default fiber statck size is 64kb
#define SHARPEN_FIBER_STACK_SIZE 64 * 1024
#endif

namespace sharpen {
    class IFiberScheduler
        : public sharpen::Noncopyable
        , public sharpen::Nonmovable {
    private:
        constexpr static std::size_t defaultFiberStackSize_{SHARPEN_FIBER_STACK_SIZE};

        virtual void NviSchedule(sharpen::FiberPtr &&fiber) = 0;

        virtual void NviScheduleSoon(sharpen::FiberPtr &&fiber) = 0;

    public:
        IFiberScheduler() noexcept = default;

        virtual ~IFiberScheduler() noexcept = default;

        inline void Schedule(sharpen::FiberPtr &&fiber) {
            fiber->SetScheduler(this);
            this->NviSchedule(std::move(fiber));
        }

        inline void ScheduleSoon(sharpen::FiberPtr &&fiber) {
            fiber->SetScheduler(this);
            this->NviScheduleSoon(std::move(fiber));
        }

        template<typename _Fn,
                 typename... _Args,
                 typename _Check = sharpen::EnableIf<
                     sharpen::IsCompletedBindableReturned<void, _Fn, _Args...>::Value>>
        void Launch(_Fn &&fn, _Args &&...args) {
            this->LaunchSpecial(
                defaultFiberStackSize_, std::forward<_Fn>(fn), std::forward<_Args>(args)...);
        }

        template<typename _Fn,
                 typename... _Args,
                 typename _Check = sharpen::EnableIf<
                     sharpen::IsCompletedBindableReturned<void, _Fn, _Args...>::Value>>
        void LaunchSpecial(std::size_t stackSize, _Fn &&fn, _Args &&...args) {
            sharpen::FiberPtr fiber = sharpen::Fiber::MakeFiber(
                stackSize, std::forward<_Fn>(fn), std::forward<_Args>(args)...);
            fiber->SetScheduler(this);
            this->Schedule(std::move(fiber));
        }

        template<typename _Fn,
                 typename... _Args,
                 typename _R,
                 typename _Check = sharpen::EnableIf<
                     sharpen::IsCompletedBindableReturned<_R, _Fn, _Args...>::Value>>
        inline void Invoke(sharpen::Future<_R> &future, _Fn &&fn, _Args &&...args) {
            using FnPtr = void (*)(sharpen::Future<_R> *, std::function<_R()>);
            FnPtr fnPtr{static_cast<FnPtr>(&sharpen::FutureCompletor<_R>::CompleteForBind)};
            std::function<_R()> task{
                std::bind(std::forward<_Fn>(fn), std::forward<_Args>(args)...)};
            this->Launch(fnPtr, &future, std::move(task));
        }

        template<typename _Fn,
                 typename... _Args,
                 typename _R,
                 typename _Check = sharpen::EnableIf<
                     sharpen::IsCompletedBindableReturned<_R, _Fn, _Args...>::Value>>
        inline void InvokeSpecial(std::size_t stackSize,
                                  sharpen::Future<_R> &future,
                                  _Fn &&fn,
                                  _Args &&...args) {
            using FnPtr = void (*)(sharpen::Future<_R> *, std::function<_R()>);
            FnPtr fnPtr{static_cast<FnPtr>(&sharpen::FutureCompletor<_R>::CompleteForBind)};
            std::function<_R()> task{
                std::bind(std::forward<_Fn>(fn), std::forward<_Args>(args)...)};
            this->LaunchSpecial(stackSize, fnPtr, &future, std::move(task));
        }


        virtual bool IsProcesser() const = 0;

        virtual void SwitchToProcesserFiber() noexcept = 0;

        virtual void SetSwitchCallback(std::function<void()> fn) = 0;

        virtual std::size_t GetParallelCount() const noexcept = 0;
    };
}   // namespace sharpen

#endif