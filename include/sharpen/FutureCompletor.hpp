#pragma once
#ifndef _SHARPEN_FUTURECOMPLETOR_HPP
#define _SHARPEN_FUTURECOMPLETOR_HPP

#include "Future.hpp"
#include <cassert>

namespace sharpen
{
    template<typename _T>
    struct FutureCompletor
    {
        template<typename _Fn,
                 typename... _Args,
                 typename _Check = decltype(_T{std::declval<_Fn>()(std::declval<_Args>()...)})>
        static void Complete(sharpen::Future<_T> &future, _Fn &&fn, _Args &&...args)
        {
            try
            {
                _T &&result{fn(std::forward<_Args>(args)...)};
                future.Complete(std::move(result));
            }
            catch (const std::exception &)
            {
                future.Fail(std::current_exception());
            }
        }

        inline static void CompleteForBind(sharpen::Future<_T> *future, std::function<_T()> fn)
        {
            assert(future != nullptr);
            assert(fn);
            try
            {
                _T &&result{fn()};
                future->Complete(std::move(result));
            }
            catch (const std::exception &)
            {
                future->Fail(std::current_exception());
            }
        }
    };

    template<>
    struct FutureCompletor<void>
    {
        template<typename _Fn,
                 typename... _Args,
                 typename _Check = decltype(std::declval<_Fn>()(std::declval<_Args>()...))>
        static void Complete(sharpen::Future<void> &future, _Fn &&fn, _Args &&...args)
        {
            try
            {
                fn(std::forward<_Args>(args)...);
                future.Complete();
            }
            catch (const std::exception &)
            {
                future.Fail(std::current_exception());
            }
        }

        inline static void CompleteForBind(sharpen::Future<void> *future, std::function<void()> fn)
        {
            assert(future != nullptr);
            assert(fn);
            try
            {
                fn();
                future->Complete();
            }
            catch (const std::exception &)
            {
                future->Fail(std::current_exception());
            }
        }
    };
}   // namespace sharpen

#endif