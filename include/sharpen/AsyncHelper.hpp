#pragma once
#ifndef _SHARPEN_ASYNCHELPER_HPP
#define _SHARPEN_ASYNCHELPER_HPP

#include <stdexcept>

#include "Future.hpp"

namespace sharpen
{
    template<typename _Fn,typename _Result>
    struct AsyncHelper
    {
        static void RunAndSetFuture(_Fn &fn,sharpen::Future<_Result> &future)
        {
            try
            {
                auto &&val = fn();
                future.Complete(std::move(val));
            }
            catch(const std::exception&)
            {
                future.Fail(std::current_exception());
            }
        }
    };

    template<typename _Fn>
    struct AsyncHelper<_Fn,void>
    {
        static void RunAndSetFuture(_Fn &fn,sharpen::Future<void> &future)
        {
            try
            {
                fn();
                future.Complete();
            }
            catch(const std::exception&)
            {
                future.Fail(std::current_exception());
            }
        }
    };
}

#endif