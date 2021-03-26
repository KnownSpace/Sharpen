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

        virtual void Select(EventVector &events) override;
        
        virtual void Notify() override;
        
        virtual void Resister(WeakChannelPtr channel) override;
        
        virtual void EnableWriteListen(sharpen::ChannelPtr channel) override;
        
        virtual void DisableWritelisten(sharpen::ChannelPtr channel) override;

        virtual void Stop() noexcept override;
    };
}

#endif
#endif
