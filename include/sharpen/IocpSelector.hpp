#pragma once
#ifndef _SHARPEN_IOCPSELECTOR_HPP
#define _SHARPEN_IOCPSELECTOR_HPP

#include "IoCompletionPort.hpp"

#ifdef SHARPEN_HAS_IOCP

#include "ISelector.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"

namespace sharpen
{
    class IocpSelector:public sharpen::ISelector,public sharpen::Nonmovable,public sharpen::Noncopyable
    {
    private:
    
        sharpen::IoCompletionPort iocp_;
    public:
    };
}

#endif
#endif
