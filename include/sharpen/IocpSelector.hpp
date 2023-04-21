#pragma once
#ifndef _SHARPEN_IOCPSELECTOR_HPP
#define _SHARPEN_IOCPSELECTOR_HPP

#include "IoCompletionPort.hpp"

#ifdef SHARPEN_HAS_IOCP

#include <vector>

#include "IChannel.hpp"
#include "ISelector.hpp"
#include "IocpOverlappedStruct.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "SpinLock.hpp"

namespace sharpen
{
    class IocpSelector
        : public sharpen::ISelector
        , public sharpen::Nonmovable
        , public sharpen::Noncopyable
    {
    private:
        using EventBuf = std::vector<sharpen::IoCompletionPort::Event>;
        using Self = sharpen::IocpSelector;

        static constexpr std::size_t minEventBufLength_{8};
        static constexpr std::size_t maxEventBufLength_{512};

        sharpen::IoCompletionPort iocp_;

        EventBuf eventBuf_;

        static bool CheckChannel(sharpen::ChannelPtr &channel) noexcept;

    public:
        IocpSelector();

        ~IocpSelector() noexcept = default;

        virtual void Select(EventVector &events) override;

        virtual void Notify() override;

        virtual void Resister(WeakChannelPtr channel) override;
    };
}   // namespace sharpen

#endif
#endif