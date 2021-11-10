#pragma once
#ifndef _SHARPEN_IPOSIXIOOPERATOR_HPP
#define _SHARPEN_IPOSIXOPERATOR_HPP

#include "SystemMacro.hpp"

#ifdef SHARPEN_IS_NIX

#include <vector>
#include <sys/uio.h>
#include <functional>

#include "TypeDef.hpp"
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

        void ConvertByteToBufferNumber(sharpen::Size byteNumber,sharpen::Size &bufferNumber,sharpen::Size &lastSize);

        void FillBufferAndCallback();

        void MoveMark(sharpen::Size newMark);

        sharpen::Size GetMark() const;

        const IoBuffer *GetFirstBuffer() const;

        IoBuffer *GetFirstBuffer();

        const Callback *GetFirstCallback() const;

        Callback *GetFirstCallback();

        sharpen::Size GetRemainingSize() const;

        sharpen::Size ComputePendingSize() const;

        virtual void DoExecute(sharpen::FileHandle handle,bool &executed,bool &blocking) = 0;
    private:
        
        sharpen::Size mark_;
    public:
        IPosixIoOperator();

        virtual ~IPosixIoOperator() noexcept = default;

        void AddPendingTask(sharpen::Char *buf,sharpen::Size size,Callback cb);

        void Execute(sharpen::FileHandle handle,bool &executed,bool &blocking);

        static bool IsBlockingError(sharpen::ErrorCode code);

        void CancelAllIo(sharpen::ErrorCode err) noexcept;
    };
}

#endif

#endif