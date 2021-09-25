#pragma once
#ifndef _SHARPEN_STOPWATCHER_HPP
#define _SHARPEN_STOPWATCHER_HPP

#include <ctime>

#include "TypeDef.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"

namespace sharpen
{
    struct StopWatcher:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        std::clock_t begin_;
        std::clock_t end_;
    public:
        StopWatcher() = default;

        ~StopWatcher() noexcept = default;

        void Begin() noexcept
        {
            this->begin_ = std::clock();
        }

        void Stop() noexcept
        {
            this->end_ = std::clock();
        }

        std::clock_t Compute() noexcept
        {
            return this->end_ - this->begin_;
        }
    };
}

#endif