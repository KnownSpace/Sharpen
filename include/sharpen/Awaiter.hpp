#pragma once
#ifndef _SHARPEN_AWAITER_HPP
#define _SHARPEN_AWAITER_HPP

#include <type_traits>
#include <cassert>

#include "Future.hpp"
#include "CoroutineEngine.hpp"

namespace sharpen
{
    template<typename _T>
    struct AwaitResult
    {
        using Result = _T&;
        using ConstResult = const _T&;
    };
    
    template<>
    struct AwaitResult<void>
    {
        using Result = void;
        using ConstResult = void;
    };
    
    template<typename _T>
    class Awaiter:public sharpen::Noncopyable
    {
    private:
        std::unique_ptr<sharpen::ExecuteContext> context_;
    public:
        Awaiter(std::unique_ptr &&context)
            :context_(std::move(context))
        {
            assert(this->context_ != nullptr);
        }
        
        ~Awaiter() = default;
        
        void operator()(sharpen::Future<_T> &future)
        {
            this->context_.SetAutoRelease(true);
            sharpen::CentralEngine.Push(std::move(this->context_));
        }
    };
}

#endif
