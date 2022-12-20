#pragma once
#ifndef _SHARPEN_IPOSIXIOOPERATOR_HPP
#define _SHARPEN_IPOSIXOPERATOR_HPP

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_NIX

#include <vector>
#include <sys/uio.h>
#include <functional>

#include <cstdint>
#include <cstddef>
#include "FileTypeDef.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "SystemError.hpp"

namespace sharpen
{
    class IPosixIoOperator : public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    protected:
        using IoBuffer = iovec;
        using IoBuffers = std::vector<IoBuffer>;
        using Callback = std::function<void(ssize_t)>;
        using Callbacks = std::vector<Callback>;

        IoBuffers bufs_;
        IoBuffers pendingBufs_;
        Callbacks cbs_;
        Callbacks pendingCbs_;

        void ConvertByteToBufferNumber(std::size_t byteNumber,std::size_t &bufferNumber,std::size_t &lastSize);

        void FillBufferAndCallback();

        void MoveMark(std::size_t newMark);

        std::size_t GetMark() const;

        const IoBuffer *GetFirstBuffer() const;

        IoBuffer *GetFirstBuffer();

        const Callback *GetFirstCallback() const;

        Callback *GetFirstCallback();

        std::size_t GetRemainingSize() const;

        std::size_t ComputePendingSize() const;

        virtual void NviExecute(sharpen::FileHandle handle,bool &executed,bool &blocking) = 0;
    private:
        
        std::size_t mark_;
    public:
        IPosixIoOperator();

        virtual ~IPosixIoOperator() noexcept = default;

        void AddPendingTask(char *buf,std::size_t size,Callback cb);

        void Execute(sharpen::FileHandle handle,bool &executed,bool &blocking);

        static bool IsBlockingError(sharpen::ErrorCode code);

        void CancelAllIo(sharpen::ErrorCode err) noexcept;
    };
}

#endif

#endif