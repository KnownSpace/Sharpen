#pragma once
#ifndef _SHARPEN_STOPWATCHER_HPP
#define _SHARPEN_STOPWATCHER_HPP

#include <ctime>
#include "TypeDef.hpp"

namespace sharpen
{
    struct StopWatcher
    {
    private:
        std::clock_t begin_;
        std::clock_t end_;
    public:
        void Begin()
        {
            this->begin_ = std::clock();
        }

        void Stop()
        {
            this->end_ = std::clock();
        }

        sharpen::Uint64 Compute()
        {
            return this->end_ - this->begin_;
        }
    };
}

#endif