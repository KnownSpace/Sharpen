#pragma once
#ifndef _SHARPEN_IFILECHANNEL_HPP
#define _SHARPEN_IFILECHANNEL_HPP

#include "IChannel.hpp"
#include "Noncopyable.hpp"
#include "IAsyncRandomWritable.hpp"
#include "IAsyncRandomReadable.hpp"
#include "FileMemory.hpp"
#include "SystemMacro.hpp"

namespace sharpen
{
    class IFileChannel:public sharpen::IChannel,public sharpen::IAsyncRandomWritable,public sharpen::IAsyncRandomReadable
    {
    private:
        using Self = sharpen::IFileChannel;

    public:

#ifdef SHARPEN_IS_WIN
        constexpr static sharpen::Size AllocationGranularity = 64*1024;
#else
        constexpr static sharpen::Size AllocationGranularity = 4*1024;
#endif
        
        IFileChannel() = default;
        
        virtual ~IFileChannel() = default;
        
        IFileChannel(const Self &) = default;
        
        IFileChannel(Self &&other) noexcept = default;

        virtual sharpen::Uint64 GetFileSize() const = 0;

        void ZeroMemoryAsync(sharpen::Future<sharpen::Size> &future,sharpen::Size size,sharpen::Uint64 offset);

        sharpen::Size ZeroMemoryAsync(sharpen::Size size,sharpen::Uint64 offset);

        inline sharpen::Size ZeroMemoryAsync(sharpen::Size size)
        {
            return this->ZeroMemoryAsync(size,0);
        }

        virtual sharpen::FileMemory MapMemory(sharpen::Size size,sharpen::Uint64 offset) = 0;
    };

    using FileChannelPtr = std::shared_ptr<sharpen::IFileChannel>;

    sharpen::FileChannelPtr MakeFileChannel(const char *filename,sharpen::FileAccessModel access,sharpen::FileOpenModel open);
}

#endif
