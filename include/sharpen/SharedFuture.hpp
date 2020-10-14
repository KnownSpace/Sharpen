#pragma once
#ifndef
#define _SHARPEN_SHAREDFUTURE_HPP

#include <memory>

#include "Future.hpp"

namespace sharpen
{
    template<typename _T>
    class SharedFuture
    {
    private:
        using Future = sharpen::Future<_T>;
        using Self = sharpen::SharedFuture<_T>;
        
        std::shared_ptr<Future> future_
    public:
    
        template<typename ..._Args>
        void Complete(_Args &&...args)
        {
            this->future.Complete(std::forward(args)...);
        }
        
        void Fail(std::exception_ptr &&exception)
        {
            this->future_.Fail(std::move(exception));
        }
        
        void Wait()
        {
            this->future_.Wait();
        }
        
        auto Get() -> decltype(this->future_.Get())
        {
            return this->future_.Get();
        }
        
        auto Get() const -> decltype(this->future.Get())
        {
            return this->future_.Get();
        }
        
        template<typename _Result>
        sharpen::SharedFuture<_Result> Then(std::function<_Result(Self)>);
    };
}

#endif
