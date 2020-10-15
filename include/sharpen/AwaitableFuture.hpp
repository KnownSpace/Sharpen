#pragma once
#ifndef _SHARPEN_AWAITABLEFUTURE_HPP
#define _SHARPEN_AWAITABLEFUTURE_HPP

#include ”Awaiter.hpp"
#include "Future.hpp"
#include "CoroutineEngine.hpp"

namespace sharpen
{
    template<typename _Result>
    class AwaitableFuture:public sharpen::Future<_Result>
    {
    };
  
    template<typename _T>
    using SharedAwaitableFuturePtr = std::shared_ptr<sharpen::AwaitableFuture<_T>>;
}

#endif
