#pragma once
#ifndef _SHARPEN_ICHANNEL_HPP
#define _SHARPEN_ICHANNEL_HPP

#include "Noncopyable.hpp"
#include "Nonmovable.hpp"

namespace sharpen
{
    class IChannel:public sharpen::Noncopyable,public sharpen::Nonmovable 
    {
    public:
        IChannel() = default;

        virtual ~IChannel() noexcept = 0;

        virtual void Close() = 0;
    };
}

#endif