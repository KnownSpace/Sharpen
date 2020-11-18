#pragma once
#ifndef _SHARPEN_TIMER_HPP
#define _SHARPEN_TIMER_HPP

#include <functional>

#include "Nonmovable.hpp"
#include "Noncopyable.hpp"
#include "SystemMacro.hpp"

namespace sharpen
{

    class Timer:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Callback = std::function<void()>;
    
        Callback callback_;
        
    public:
    };
}

#endif
