#pragma once
#ifndef _SHARPEN_IFILECHANNEL_HPP
#define _SHARPEN_IFILECHANNEL_HPP

#include "IChannel.hpp"
#include "Noncopyable.hpp"
#include "IAsyncRandomWritable.hpp"
#include "IAsyncRandomReadable.hpp"

namespace sharpen
{
    class IFileChannel:public sharpen::IChannel,public sharpen::IAsyncRandomWritable,public sharpen::IAsyncRandomReadable
    {
    private:
        using Self = sharpen::IFileChannel;

    public:
        
        IFileChannel() = default;
        
        virtual ~IFileChannel() = default;
        
        IFileChannel(const Self &) = default;
        
        IFileChannel(Self &&other) noexcept = default;

        virtual sharpen::Uint64 GetFileSize() const = 0;

        void ZeroMemoryAsync(sharpen::Future<sharpen::Size> &future,sharpen::Size size,sharpen::Uint64 offset);

        sharpen::Size ZeroMemoryAsync(sharpen::Size size,sharpen::Uint64 offset);
    };

    using FileChannelPtr = std::shared_ptr<sharpen::IFileChannel>;

    sharpen::FileChannelPtr MakeFileChannel(const char *filename,sharpen::FileAccessModel access,sharpen::FileOpenModel open);
}

#endif
