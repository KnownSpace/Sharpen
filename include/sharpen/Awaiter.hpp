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
        using Self = sharpen::Awaiter<_T>;
    
        std::unique_ptr<sharpen::ExecuteContext> context_;
    public:
        Awaiter() = default;

        explicit Awaiter(std::unique_ptr<sharpen::ExecuteContext> &&context) noexcept
            :context_(std::move(context))
        {
            assert(this->context_ != nullptr);
        }
        
        Awaiter(Self &&other) noexcept
            :context_(std::move(other.context_))
        {
            assert(this->context_ != nullptr);
        }
        
        ~Awaiter() noexcept = default;
        
        void operator()(sharpen::Future<_T> &future) noexcept
        {
            this->context_->SetAutoRelease(true);
            sharpen::CentralEngine.PushContext(std::move(this->context_));
        }
        
        sharpen::ExecuteContext &GetContext() noexcept
        {
            return *(this->context_);
        }

        Self &operator=(Self &&other) noexcept
        {
            this->context_ = std::move(other.context_);
            return *this;
        }
    };
}

#endif
