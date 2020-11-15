#pragma once
#ifndef _SHARPEN_EPOLLSELECTOR_HPP
#define _SHARPEN_EPOLLSELECTOR_HPP

#include "Epoll.hpp"

#ifdef SHARPEN_HAS_EPOLL

#include "ISelector.hpp"
#include "EventFd.hpp"
#include "Nonmovable.hpp"

namespace sharpen
{
    class EpollSelector:public sharpen::ISelector,public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
    
        sharpen::Epoll epoll_;
    
        sharpen::EventFd eventfd_;
    public:
    };
}

#endif
#endif
