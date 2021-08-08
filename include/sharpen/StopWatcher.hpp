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