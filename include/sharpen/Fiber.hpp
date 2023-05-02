#pragma once
#ifndef _SHARPEN_FIBER_HPP
#define _SHARPEN_FIBER_HPP

#include "FiberLocalStorage.hpp"
#include "MemoryStack.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "TypeTraits.hpp"
#include <cassert>
#include <functional>
#include <memory>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *fcontext_t;

typedef struct {
    fcontext_t fctx;
    void *data;
} transfer_t;

#ifdef __cplusplus
}
#endif

namespace sharpen {

    class Fiber;

    using FiberPtr = std::shared_ptr<sharpen::Fiber>;

    class IFiberScheduler;

    class Fiber
        : public sharpen::Noncopyable
        , public sharpen::Nonmovable
        , public std::enable_shared_from_this<sharpen::Fiber> {
    private:
        using Handle = fcontext_t;
        using Task = std::function<void()>;
        using Callback = sharpen::Fiber *;

        // fcontext
        Handle handle_;

        // stack
        MemoryStack stack_;

        // task
        Task task_;

        // callback
        Callback callback_;

        // inited
        bool inited_;

        // scheduler
        sharpen::IFiberScheduler *scheduler_;

        // local storage
        sharpen::FiberLocalStorage localStorage_;

        thread_local static FiberPtr currentFiber_;

        static void FiberEntry(transfer_t from) noexcept;

        static transfer_t SaveCurrentAndSwitch(transfer_t from) noexcept;

        void InitFiber();

    public:
        Fiber() noexcept;

        ~Fiber() noexcept;

        void Switch() noexcept;

        void Switch(sharpen::Fiber *callback) noexcept;

        static sharpen::FiberPtr GetCurrentFiber() noexcept;

        static sharpen::IFiberScheduler *GetCurrentFiberSceduler() noexcept;

        void Release() noexcept;

        sharpen::IFiberScheduler *GetScheduler() const noexcept;

        void SetScheduler(sharpen::IFiberScheduler *scheduler) noexcept;

        template<typename _Fn,
                 typename... _Args,
                 typename _Check = sharpen::EnableIf<
                     sharpen::IsCompletedBindableReturned<void, _Fn, _Args...>::Value>>
        static sharpen::FiberPtr MakeFiber(std::size_t stackSize, _Fn &&fn, _Args &&...args) {
            sharpen::FiberPtr fiber = std::make_shared<sharpen::Fiber>();
            fiber->stack_ = sharpen::MemoryStack(nullptr, stackSize);
            fiber->task_ =
                std::move(std::bind(std::forward<_Fn>(fn), std::forward<_Args>(args)...));
            return fiber;
        }

        inline sharpen::FiberLocalStorage &LocalStorage() noexcept {
            return this->localStorage_;
        }

        inline const sharpen::FiberLocalStorage &LocalStorage() const noexcept {
            return this->localStorage_;
        }

        static sharpen::FiberLocalStorage &GetLocalStorage() noexcept;
    };

    extern sharpen::IFiberScheduler *GetLocalSchedulerPtr() noexcept;

    inline sharpen::IFiberScheduler &GetLocalScheduler() noexcept {
        sharpen::IFiberScheduler *scheduler{sharpen::GetLocalSchedulerPtr()};
        assert(scheduler != nullptr);
        return *scheduler;
    }
}   // namespace sharpen

#endif