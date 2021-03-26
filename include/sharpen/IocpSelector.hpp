#pragma once
#ifndef _SHARPEN_IOCPSELECTOR_HPP
#define _SHARPEN_IOCPSELECTOR_HPP

#include "IoCompletionPort.hpp"

#ifdef SHARPEN_HAS_IOCP

#include <unordered_map>

#include "ISelector.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "SpinLock.hpp"
#include "IocpOverlappedStruct.hpp"

namespace sharpen
{
    class IocpSelector:public sharpen::ISelector,public sharpen::Nonmovable,public sharpen::Noncopyable
    {
    private:
        sharpen::IoCompletionPort iocp_;

        sharpen::Size count_;

        static bool CheckChannel(sharpen::ChannelPtr &channel) noexcept;
    public:

        IocpSelector();

        ~IocpSelector() noexcept = default;

        virtual void Select(EventVector &events) override;
        
        virtual void Notify() override;
        
        virtual void Resister(WeakChannelPtr channel) override;
        
        virtual void EnableWriteListen(sharpen::ChannelPtr channel) override;
        
        virtual void DisableWritelisten(sharpen::ChannelPtr channel) override;
    };
}

#endif
#endif