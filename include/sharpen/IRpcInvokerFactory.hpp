#pragma once
#ifndef _SHARPEN_IRPCINVOKERFACTORY_HPP
#define _SHARPEN_IRPCINVOKERFACTORY_HPP

#include "IEndPoint.hpp"
#include "IRpcInvoker.hpp"

namespace sharpen
{
    class IRpcInvokerFactory
    {
    private:
        using Self = sharpen::IRpcInvokerFactory;
    protected:
    public:
    
        IRpcInvokerFactory() noexcept = default;
    
        IRpcInvokerFactory(const Self &other) noexcept = default;
    
        IRpcInvokerFactory(Self &&other) noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    
        virtual ~IRpcInvokerFactory() noexcept = default;

        virtual std::unique_ptr<sharpen::IRpcInvoker> CreateInvoker(const sharpen::IEndPoint &endpoint) = 0;
    };
}

#endif