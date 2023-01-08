#pragma once
#ifndef _SHARPEN_POSIXPIPESIGNALCHANNEL_HPP
#define _SHARPEN_POSIXPIPESIGNALCHANNEL_HPP

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_NIX

#define SHARPEN_HAS_POSIXPIPESIGNALCHANNEL

#include "ISignalChannel.hpp"
#include "PosixIoReader.hpp"

namespace sharpen
{
    class PosixPipeSignalChannel:public sharpen::ISignalChannel,public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Self = sharpen::PosixPipeSignalChannel;
        using Callback = std::function<void(ssize_t)>;
        using Base = sharpen::ISignalChannel;
    
        sharpen::FileHandle writer_;
        sharpen::SignalMap *map_;
        sharpen::PosixIoReader reader_;
        bool readable_;

        sharpen::FileHandle GetReader() const noexcept;

        sharpen::FileHandle GetWriter() const noexcept;

        static void DoClose(sharpen::FileHandle handle,sharpen::FileHandle writer,sharpen::SignalMap *map) noexcept;

        void HandleRead();

        void DoRead();

        void TryRead(char *buf,std::size_t bufSize,Callback cb);

        void RequestRead(char *buf,std::size_t bufSize,sharpen::Future<std::size_t> *future);

        static void CompleteReadCallback(sharpen::EventLoop *loop,sharpen::Future<std::size_t> *future,ssize_t size) noexcept;
    public:
    
        PosixPipeSignalChannel(sharpen::FileHandle reader,sharpen::FileHandle writer,sharpen::SignalMap &map);
    
        virtual ~PosixPipeSignalChannel() noexcept;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        virtual void ReadAsync(sharpen::SignalBuffer &signals,sharpen::Future<std::size_t> &future) override;

        virtual void OnEvent(sharpen::IoEvent *event) override;
    };
}

#endif
#endif