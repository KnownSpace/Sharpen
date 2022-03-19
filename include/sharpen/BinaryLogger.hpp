#pragma once
#ifndef _SHARPEN_BINARYLOGGER_HPP
#define _SHARPEN_BINARYLOGGER_HPP

/*
+---------------------+
| Batch Size1         | 8 bytes
+---------------------+
| Batch1              |
+---------------------+
|    ...              |
+---------------------+
| Batch SizeN         | 8 bytes
+---------------------+
| BatchN              |
+---------------------+
*/

#include "Noncopyable.hpp"
#include "IFileChannel.hpp"
#include "CompilerInfo.hpp"
#include "ByteOrder.hpp"
#include "WriteBatch.hpp"
#include "IntOps.hpp"

namespace sharpen
{
    class BinaryLogger:public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::BinaryLogger;
    
        sharpen::FileChannelPtr channel_;
        std::atomic_uint64_t offset_;
    public:
        BinaryLogger(sharpen::FileChannelPtr channel);
    
        BinaryLogger(Self &&other) noexcept
            :channel_(std::move(other.channel_))
            ,offset_(other.offset_.load())
        {}
    
        Self &operator=(Self &&other) noexcept;
    
        ~BinaryLogger() noexcept = default;

        void Log(const sharpen::WriteBatch &batch);

        std::vector<sharpen::WriteBatch> GetWriteBatchs();

        void Clear();
    };
}

#endif