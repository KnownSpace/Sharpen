#pragma once
#ifndef _SHARPEN_IWORKERGROUP_HPP
#define _SHARPEN_IWORKERGROUP_HPP

#include <functional>

#include "AwaitableFuture.hpp"
#include "FutureCompletor.hpp"
#include "TypeTraits.hpp"

namespace sharpen
{
    class IWorkerGroup
    {
    private:
        using Self = sharpen::IWorkerGroup;

    protected:
        virtual void NviSubmit(std::function<void()> task) = 0;

    public:
        IWorkerGroup() noexcept = default;

        IWorkerGroup(const Self &other) noexcept = default;

        IWorkerGroup(Self &&other) noexcept = default;

        Self &operator=(const Self &other) noexcept = default;

        Self &operator=(Self &&other) noexcept = default;

        virtual ~IWorkerGroup() noexcept = default;

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        virtual void Stop() noexcept = 0;

        virtual void Join() noexcept = 0;

        virtual bool Running() const noexcept = 0;

        virtual std::size_t GetWorkerCount() const noexcept = 0;

        template<typename _Fn,
                 typename... _Args,
                 typename _Check = sharpen::EnableIf<
                     sharpen::IsCompletedBindableReturned<void, _Fn, _Args...>::Value>>
        inline void Submit(_Fn &&fn, _Args &&...args)
        {
            std::function<void()> task{
                std::bind(std::forward<_Fn>(fn), std::forward<_Args>(args)...)};
            this->NviSubmit(std::move(task));
        }

        template<typename _Fn,
                 typename... _Args,
                 typename _R,
                 typename _Check = sharpen::EnableIf<
                     sharpen::IsCompletedBindableReturned<_R, _Fn, _Args...>::Value>>
        inline void Invoke(sharpen::Future<_R> &future, _Fn &&fn, _Args &&...args)
        {
            assert(future.IsPending());
            using FnPtr = void (*)(sharpen::Future<_R> *, std::function<_R()>);
            FnPtr fnPtr{static_cast<FnPtr>(&sharpen::FutureCompletor<_R>::CompleteForBind)};
            std::function<_R()> task{
                std::bind(std::forward<_Fn>(fn), std::forward<_Args>(args)...)};
            std::function<void()> packagedTask{std::bind(fnPtr, &future, std::move(task))};
            this->NviSubmit(std::move(packagedTask));
        }

        template<typename _Fn,
                 typename... _Args,
                 typename _R = decltype(std::declval<_Fn>()(std::declval<_Args>()...)),
                 typename _Check = sharpen::EnableIf<
                     sharpen::IsCompletedBindableReturned<_R, _Fn, _Args...>::Value>>
        inline sharpen::AwaitableFuturePtr<_R> Invoke(_Fn &&fn, _Args &&...args)
        {
            sharpen::AwaitableFuturePtr<_R> future{sharpen::MakeAwaitableFuture<_R>()};
            this->Invoke(*future, std::forward<_Fn>(fn), std::forward<_Args>(args)...);
            return future;
        }
    };
}   // namespace sharpen

#endif