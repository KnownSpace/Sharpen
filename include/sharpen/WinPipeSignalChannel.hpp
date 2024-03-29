#pragma once
#ifndef _SHARPEN_WINPIPESIGNALCHANNEL_HPP
#define _SHARPEN_WINPIPESIGNALCHANNEL_HPP

#include "ISignalChannel.hpp"
#include "IoEvent.hpp"
#include "SystemError.hpp"

#ifdef SHARPEN_IS_WIN
#include "IocpSelector.hpp"

#define SHARPEN_HAS_WINSIGNALPIEPECHANNEL

namespace sharpen {
    class WinPipeSignalChannel
        : public sharpen::ISignalChannel
        , public sharpen::Noncopyable
        , public sharpen::Nonmovable {
    private:
        using Self = sharpen::WinPipeSignalChannel;
        using Base = sharpen::ISignalChannel;

        sharpen::FileHandle GetWriter() const noexcept;

        sharpen::FileHandle GetReader() const noexcept;

        static void DoClose(sharpen::FileHandle handle,
                            sharpen::FileHandle writer,
                            sharpen::SignalMap *map) noexcept;

        sharpen::SignalMap *map_;
        sharpen::FileHandle writer_;

        static void InitOverlapped(OVERLAPPED &ol);

        void InitOverlappedStruct(sharpen::IocpOverlappedStruct &event);

        void RequestRead(char *sigs, std::size_t size, sharpen::Future<std::size_t> *future);

    public:
        WinPipeSignalChannel(sharpen::FileHandle reader,
                             sharpen::FileHandle writer,
                             sharpen::SignalMap &map);

        virtual ~WinPipeSignalChannel() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        virtual void ReadAsync(sharpen::SignalBuffer &signals,
                               sharpen::Future<std::size_t> &future) override;

        virtual void OnEvent(sharpen::IoEvent *event) override;
    };
}   // namespace sharpen
#endif
#endif