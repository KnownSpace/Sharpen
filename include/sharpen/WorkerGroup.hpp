#pragma once
#ifndef _SHARPEN_WORKERGROUP_HPP
#define _SHARPEN_WORKERGROUP_HPP

#include <vector>

#include "AwaitableFuture.hpp"
#include "AsyncBlockingQueue.hpp"
#include "NoexceptInvoke.hpp"
#include "TypeTraits.hpp"
#include "FutureCompletor.hpp"

namespace sharpen
{
    class EventEngine;

    class WorkerGroup:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Self = sharpen::WorkerGroup;

        void Entry(std::size_t index);

        std::atomic_bool token_;
        sharpen::AsyncBlockingQueue<std::function<void()>> queue_;
        std::vector<sharpen::AwaitableFuture<void>> workers_;
    public:
    
        explicit WorkerGroup(sharpen::EventEngine &engine);

        WorkerGroup(sharpen::EventEngine &engine,std::size_t workerCount);
    
        ~WorkerGroup() noexcept
        {
            this->Stop();
            this->Join();
        }
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        void Join() noexcept;

        void Stop() noexcept;

        inline bool Running() const noexcept
        {
            return this->token_.load();
        }

        inline std::size_t GetSize() const noexcept
        {
            return this->workers_.size();
        }

        template<typename _Fn,typename ..._Args,typename _Check = sharpen::EnableIf<sharpen::IsCompletedBindableReturned<void,_Fn,_Args...>::Value>>
        inline void Submit(_Fn &&fn,_Args &&...args)
        {
            assert(this->token_.load());
            std::function<void()> task{std::forward<_Fn>(fn),std::forward<_Args>(args)...};
            this->queue_.Emplace(std::move(task));
        }

        template<typename _Fn,typename ..._Args,typename _R,typename _Check = sharpen::EnableIf<sharpen::IsCompletedBindableReturned<_R,_Fn,_Args...>::Value>>
        inline void Invoke(sharpen::Future<_R> &future,_Fn &&fn,_Args &&...args)
        {
            assert(this->token_.load());
            assert(future.IsPending());
            using FnPtr = void(*)(sharpen::Future<_R>*,std::function<_R()>);
            FnPtr fnPtr{static_cast<FnPtr>(&sharpen::FutureCompletor<_R>::CompleteForBind)};
            std::function<_R()> task{std::bind(std::forward<_Fn>(fn),std::forward<_Args>(args)...)};
            std::function<void()> packagedTask{std::bind(fnPtr,&future,std::move(task))};
            this->queue_.Emplace(std::move(packagedTask));
        }

        template<typename _Fn,typename ..._Args,typename _R = decltype(std::declval<_Fn>()(std::declval<_Args>()...)),typename _Check = sharpen::EnableIf<sharpen::IsCompletedBindableReturned<_R,_Fn,_Args...>::Value>>
        inline sharpen::AwaitableFuturePtr<_R> Invoke(_Fn &&fn,_Args &&...args)
        {
            sharpen::AwaitableFuturePtr<_R> future{sharpen::MakeAwaitableFuture<_R>()};
            this->Invoke(*future,std::forward<_Fn>(fn),std::forward<_Args>(args)...);
            return future;
        }
    };   
}

#endif