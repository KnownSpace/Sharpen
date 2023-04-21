#pragma once
#ifndef _SHARPEN_WININPUTPIPECHANNEL_HPP
#define _SHARPEN_WININPUTPIPECHANNEL_HPP

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_WIN

#include "IInputPipeChannel.hpp"
#include "IocpSelector.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"

#define SHARPEN_HAS_WININPUTPIPE

namespace sharpen
{
    class WinInputPipeChannel
        : public sharpen::IInputPipeChannel
        , public sharpen::Noncopyable
        , public sharpen::Nonmovable
    {
    private:
        using Mybase = sharpen::IInputPipeChannel;
        using Self = sharpen::WinInputPipeChannel;

        static void InitOverlapped(OVERLAPPED &ol);

        void InitOverlappedStruct(sharpen::IocpOverlappedStruct &event);

        void RequestRead(char *buf, std::size_t bufSize, sharpen::Future<std::size_t> *future);

    public:
        explicit WinInputPipeChannel(sharpen::FileHandle handle);

        virtual ~WinInputPipeChannel() noexcept;

        virtual void ReadAsync(char *buf,
                               std::size_t bufSize,
                               sharpen::Future<std::size_t> &future) override;

        virtual void ReadAsync(sharpen::ByteBuffer &buf,
                               std::size_t bufferOffset,
                               sharpen::Future<std::size_t> &future) override;

        virtual void OnEvent(sharpen::IoEvent *event) override;
    };
}   // namespace sharpen

#endif
#endif