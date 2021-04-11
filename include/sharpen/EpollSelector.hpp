#pragma once
#ifndef _SHARPEN_EPOLLSELECTOR_HPP
#define _SHARPEN_EPOLLSELECTOR_HPP

#include "Epoll.hpp"

#ifdef SHARPEN_HAS_EPOLL

#include <map>

#include "ISelector.hpp"
#include "EventFd.hpp"
#include "Nonmovable.hpp"
#include "EpollEventStruct.hpp"
#include "SpinLock.hpp"

namespace sharpen
{
    class EpollSelector:public sharpen::ISelector,public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Event = sharpen::EpollEventStruct;
        using Map = std::map<sharpen::FileHandle,Event>;
        using Lock = sharpen::SpinLock;
    
        sharpen::Epoll epoll_;
    
        sharpen::EventFd eventfd_;

        Map map_;
        
        Lock lock_;

        sharpen::Size count_;

        static bool CheckChannel(sharpen::ChannelPtr channel) noexcept;
    public:

        EpollSelector();

        ~EpollSelector() noexcept = default;

        virtual void Select(EventVector &events) override;
        
        virtual void Notify() override;
        
        virtual void Resister(WeakChannelPtr channel) override;
    };
}

#endif
#endif
