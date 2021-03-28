#pragma once
#ifndef _SHARPEN_FIBERPROCESSERPOOL_HPP
#define _SHARPEN_FIBERPROCESSERPOOL_HPP

#include <vector>

#include "FiberProcesser.hpp"

namespace sharpen
{
    class FiberProcesserPool:public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Processers = std::vector<sharpen::FiberProcesser>;

        Processers processers_;

        bool running_;
    public:
        FiberProcesserPool();

        explicit FiberProcesserPool(sharpen::Size size);

        ~FiberProcesserPool() noexcept;

        void Stop() noexcept;

        sharpen::Size GetSize() const noexcept;
    };
    
}

#endif