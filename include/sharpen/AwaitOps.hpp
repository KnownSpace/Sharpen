#pragma once
#ifndef _SHARPEN_AWAITOPS_HPP
#define _SHARPEN_AWAITOPS_HPP

#include "AwaitableFuture.hpp"
#include <tuple>
#include <type_traits>

namespace sharpen
{
    template<typename _T>
    struct AwaitPackageHelper
    {
        using Type = _T;

        static Type Decltype();
    };

    template<>
    struct AwaitPackageHelper<void>
    {
        using Type = decltype(std::ignore);

        static Type Decltype();
    };

    template<typename _Tuple, size_t _Index, typename _MainFuture, typename... _Futures>
    struct AwaitSetTupleHelper
    {
    private:
    public:
        static void AwaitAndSet(_Tuple &tuple, _MainFuture &mainFuture, _Futures &...futures)
        {
            // do sth
            std::get<_Index>(tuple) = mainFuture.Await();
            // continue
            sharpen::AwaitSetTupleHelper<_Tuple, _Index + 1, _Futures...>::AwaitAndSet(tuple,
                                                                                       futures...);
        }
    };

    template<typename _Tuple, size_t _Index, typename _MainFuture>
    struct AwaitSetTupleHelper<_Tuple, _Index, _MainFuture>
    {
    private:
    public:
        static void AwaitAndSet(_Tuple &tuple, _MainFuture &mainFuture)
        {
            std::get<_Index>(tuple) = mainFuture.Await();
        }
    };

    template<typename _Tuple, size_t _Index, typename... _Futures>
    struct AwaitSetTupleHelper<_Tuple, _Index, sharpen::AwaitableFuture<void>, _Futures...>
    {
    private:
    public:
        static void AwaitAndSet(_Tuple &tuple,
                                sharpen::AwaitableFuture<void> &mainFuture,
                                _Futures &...futures)
        {
            mainFuture.Await();
            // continue
            sharpen::AwaitSetTupleHelper<_Tuple, _Index + 1, _Futures...>::AwaitAndSet(tuple,
                                                                                       futures...);
        }
    };

    template<typename _Tuple, size_t _Index>
    struct AwaitSetTupleHelper<_Tuple, _Index, sharpen::AwaitableFuture<void>>
    {
    private:
    public:
        static void AwaitAndSet(_Tuple &tuple, sharpen::AwaitableFuture<void> &mainFuture)
        {
            (void)tuple;
            (void)mainFuture;
            mainFuture.Await();
        }
    };

    template<typename... _T,
             typename _Ret =
                 typename std::tuple<decltype(sharpen::AwaitPackageHelper<_T>::Decltype())...>>
    inline auto AwaitAll(sharpen::AwaitableFuture<_T> &...futures) -> _Ret
    {
        _Ret tuple;
        sharpen::AwaitSetTupleHelper<_Ret, 0, sharpen::AwaitableFuture<_T>...>::AwaitAndSet(
            tuple, futures...);
        return tuple;
    }

    template<typename _MainFuture, typename... _Futures>
    struct AwaitAnyHelper
    {
    private:
        using Self = sharpen::AwaitAnyHelper<_MainFuture, _Futures...>;

        static void Callback(std::shared_ptr<std::atomic_flag> token,
                             sharpen::Future<void> *future,
                             _MainFuture &)
        {
            bool t = token->test_and_set();
            if (!t)
            {
                future->Complete();
            }
        }

    public:
        static void SetCallback(std::shared_ptr<std::atomic_flag> token,
                                sharpen::AwaitableFuture<void> &future,
                                _MainFuture &mainFuture,
                                _Futures &...futures)
        {
            using FnPtr =
                void (*)(std::shared_ptr<std::atomic_flag>, sharpen::Future<void> *, _MainFuture &);
            mainFuture.SetCallback(std::bind(
                static_cast<FnPtr>(&Self::Callback), token, &future, std::placeholders::_1));
            if (future.IsPending())
            {
                sharpen::AwaitAnyHelper<_Futures...>::SetCallback(
                    std::move(token), future, futures...);
            }
        }
    };

    template<typename _Future>
    struct AwaitAnyHelper<_Future>
    {
    private:
        using Self = sharpen::AwaitAnyHelper<_Future>;

        static void Callback(std::shared_ptr<std::atomic_flag> token,
                             sharpen::Future<void> *future,
                             _Future &)
        {
            bool t = token->test_and_set();
            if (!t)
            {
                future->Complete();
            }
        }

    public:
        static void SetCallback(std::shared_ptr<std::atomic_flag> token,
                                sharpen::AwaitableFuture<void> &future,
                                _Future &last)
        {
            using FnPtr =
                void (*)(std::shared_ptr<std::atomic_flag>, sharpen::Future<void> *, _Future &);
            last.SetCallback(std::bind(static_cast<FnPtr>(&Self::Callback),
                                       std::move(token),
                                       &future,
                                       std::placeholders::_1));
        }
    };


    template<typename... _T>
    inline void AwaitAny(sharpen::Future<_T> &...futures)
    {
        std::shared_ptr<std::atomic_flag> flag = std::make_shared<std::atomic_flag>();
        sharpen::AwaitableFuture<void> future;
        sharpen::AwaitAnyHelper<sharpen::Future<_T>...>::SetCallback(flag, future, futures...);
        future.Await();
    }
}   // namespace sharpen

#endif