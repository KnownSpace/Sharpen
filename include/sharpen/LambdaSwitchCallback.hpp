#pragma once
#ifndef _SHARPEN_LAMBDASWITCHCALLBACK_HPP
#define _SHARPEN_LAMBDASWITCHCALLBACK_HPP

#include <functional>

#include "IContextSwitchCallback.hop"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"

namespace sharpen
{
    class LambdaSwitchCallback:public sharpen::IContextSwitchCallback,public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Function = std::function<void()>;
        
        Function fn_;
    public:
        LambdaSwitchCallback(Function &&fn)
            :fn_(std::move(fn))
        {}
        
        virtual ~LambdaSwitchCallback() noexcept = default;
        
        virtual void Run() noexcept override
        {
            this->fn_();   
        }
    };
}

#endif
