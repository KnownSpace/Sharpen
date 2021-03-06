#pragma once
#ifndef _SHARPEN_KQUEUESELECTOR_HPP
#define _SHARPEN_KQUEUESELECTOR_HPP

#include "Kqueue.hpp"

#ifdef SHARPEN_HAS_KQUEUE

#include "ISelector.hpp"
#include "Pipe.hpp"
#include "Nonmovable.hpp"

namespace sharpen
{
    class KqueueSelector:public sharpen::ISelector,public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
    
        sharpen::Kqueue kqueue_;
        
        sharpen::Pipe pipe_;
    public:

        KqueueSelector();

        ~KqueueSelector() = default;

        virtual void Select(EventVector &events) override;
        
        virtual void Notify() override;
        
        virtual void Resister(WeakChannelPtr channel) override;
    };
}

#endif
#endif
