#pragma once
#ifndef _SHARPEN_WORKER_HPP
#define _SHARPEN_WORKER_HPP

#include <thread>

#include "Noncopyable.hpp"
#include "CoroutineEngine.hpp"
#include "Nonmovable.hpp"

namespace sharpen
{
    class Worker:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
    
        std::thread thread_;
        bool running_;
    public:
        Worker();
        
        ~Worker() noexcept;
        
        void Stop();
    };
}

#endif
