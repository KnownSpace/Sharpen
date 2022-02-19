#pragma once
#ifndef _SHARPEN_PERSISTENTLOGGER_HPP
#define _SHARPEN_PERSISTENTLOGGER_HPP

#include "IFileChannel.hpp"
#include "MemoryTable.hpp"
#include "BinaryLogger.hpp"
#include "PersistentTable.hpp"

namespace sharpen
{
    class PersistentLogger:public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::PersistentLogger;
        using MemLogger = sharpen::MemoryTable<sharpen::BinaryLogger>;

        static constexpr sharpen::Size memLoggerMaxSize_{64*1024*1024};
    
        sharpen::FileChannelPtr logChannel_;
        sharpen::FileChannelPtr tableChannel_;
        MemLogger memLogger_;
        sharpen::Optional<sharpen::PersistentTable> persLogger_;
    public:
    
        explicit PersistentLogger(const char *logName);
    
        PersistentLogger(Self &&other) noexcept;
    
        Self &operator=(Self &&other) noexcept;
    
        ~PersistentLogger() noexcept = default;
    };   
}

#endif