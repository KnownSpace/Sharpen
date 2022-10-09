#pragma once
#ifndef _SHARPEN_IRPCINVOKER_HPP
#define _SHARPEN_IRPCINVOKER_HPP

#include "ByteBuffer.hpp"
#include "Future.hpp"

namespace sharpen
{
    class IRpcInvoker
    {
    private:
        using Self = sharpen::IRpcInvoker;
    protected:
    public:
    
        IRpcInvoker() noexcept = default;
    
        IRpcInvoker(const Self &other) noexcept = default;
    
        IRpcInvoker(Self &&other) noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    
        virtual ~IRpcInvoker() noexcept = default;

        virtual sharpen::ByteBuffer Invoke(const sharpen::ByteBuffer &message) = 0;

        virtual void Cancel();
    };   
}

#endif