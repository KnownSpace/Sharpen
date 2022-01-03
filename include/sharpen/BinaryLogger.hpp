#pragma once
#ifndef _SHARPEN_BINARYLOGGER_HPP
#define _SHARPEN_BINARYLOGGER_HPP

#include "Noncopyable.hpp"
#include "IFileChannel.hpp"
#include "CompilerInfo.hpp"
#include "ByteOrder.hpp"

namespace sharpen
{
    struct BinaryLoggerHelper
    {
        static constexpr char PutOp_ = 0;
        static constexpr char DeleteOp = 1;

        enum class RestoreStep
        {
            ReadOp,
            ReadKeySize,
            ReadValueSize,
            ReadKey,
            ReadValue,
            Error
        };
    };
    

    template<template<class,class,class ...> class _Map>
    class BinaryLogger:public sharpen::Noncopyable
    {
    private:
        using Self = BinaryLogger;
        using MapType = _Map<sharpen::ByteBuffer,sharpen::ByteBuffer>;
    
        sharpen::FileChannelPtr channel_;
        sharpen::Uint64 offset_;
        
#ifdef SHARPEN_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable:26819)
#elif (defined SHARPEN_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#elif (defined SHARPEN_COMPILER_CLANG)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-fallthrough"
#endif

        static void RunStateMachine(const char *begin,const char *end,sharpen::BinaryLoggerHelper::RestoreStep &step,char &op,sharpen::ByteBuffer &key,sharpen::ByteBuffer &val,sharpen::Size &lenBuf,sharpen::Size &counter,MapType &map)
        {
            while (begin != end)
            {
                switch (step)
                {
                case sharpen::BinaryLoggerHelper::RestoreStep::ReadOp:
                    op = *begin++;
                    if (op == sharpen::BinaryLoggerHelper::PutOp_ || op == sharpen::BinaryLoggerHelper::DeleteOp)
                    {
                        step = sharpen::BinaryLoggerHelper::RestoreStep::ReadKeySize;
                        counter = 8;
                        lenBuf = 0;
                        continue;
                    }
                    step = sharpen::BinaryLoggerHelper::RestoreStep::Error;
                    return;
                case sharpen::BinaryLoggerHelper::RestoreStep::ReadKeySize:
                    if(counter)
                    {
                        lenBuf <<= 8;
                        lenBuf |= static_cast<unsigned char>(*begin++);
                        if(--counter)
                        {
                            continue;
                        }
                    }
                    sharpen::ConvertEndian(lenBuf);
                    key.ExtendTo(lenBuf);
                    if(op == sharpen::BinaryLoggerHelper::PutOp_)
                    {
                        counter = 8;
                        lenBuf = 0;
                        step = sharpen::BinaryLoggerHelper::RestoreStep::ReadValueSize;
                    }
                    else
                    {
                        step = sharpen::BinaryLoggerHelper::RestoreStep::ReadKey;
                    }
                    continue;
                case sharpen::BinaryLoggerHelper::RestoreStep::ReadValueSize:
                    if(counter)
                    {
                        lenBuf <<= 8;
                        lenBuf |= static_cast<unsigned char>(*begin++);
                        if(--counter)
                        {
                            continue;
                        }
                    }
                    sharpen::ConvertEndian(lenBuf);
                    val.ExtendTo(lenBuf);
                    step = sharpen::BinaryLoggerHelper::RestoreStep::ReadKey;
                    continue;
                case sharpen::BinaryLoggerHelper::RestoreStep::ReadKey:
                    if(counter != key.GetSize())
                    {
                        key[counter] = *begin++;
                        if(++counter != key.GetSize())
                        {
                            continue;
                        }
                    }
                    counter = 0;
                    if(op == sharpen::BinaryLoggerHelper::DeleteOp)
                    {
                        auto ite = map.find(key);
                        if(ite != map.end())
                        {
                            ite->second.Clear();
                            ite->second.Shrink();
                        }
                        step = sharpen::BinaryLoggerHelper::RestoreStep::ReadOp;
                        continue;
                    }
                    step = sharpen::BinaryLoggerHelper::RestoreStep::ReadValue;
                    continue;
                case sharpen::BinaryLoggerHelper::RestoreStep::ReadValue:
                    if(counter != val.GetSize())
                    {
                        val[counter++] = *begin++;
                        if(counter != val.GetSize())
                        {
                            continue;
                        }
                    }
                    map[std::move(key)] = std::move(val);
                    step = sharpen::BinaryLoggerHelper::RestoreStep::ReadOp;
                    counter = 0;
                    continue;
                }
            }
        }

#ifdef SHARPEN_COMPILER_MSVC
#pragma warning(pop)
#elif (defined SHARPEN_COMPILER_GCC)
#pragma GCC diagnostic push
#elif (defined SHARPEN_COMPILER_CLANG)
#pragma clang diagnostic pop
#endif

    public:
        explicit BinaryLogger(sharpen::FileChannelPtr channel)
            :channel_(std::move(channel))
            ,offset_(0)
        {
            this->offset_ = this->channel_->GetFileSize();
        }
    
        BinaryLogger(Self &&other) noexcept
            :channel_(std::move(other.channel_))
            ,offset_(other.offset_)
        {
            other.offset_ = 0;
        }
    
        Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->channel_ = std::move(other.channel_);
                this->offset_ = other.offset_;
                other.offset_ = 0;
            }
            return *this;
        }
    
        ~BinaryLogger() noexcept = default;

        void LogPut(const sharpen::ByteBuffer &key,const sharpen::ByteBuffer &val)
        {
            if(!key.GetSize())
            {
                throw std::logic_error("empty key");
            }
            sharpen::Uint64 len = key.GetSize();
            char buf[1+2*sizeof(len)] = {0};
            buf[0] = sharpen::BinaryLoggerHelper::PutOp_;
            std::memcpy(buf + 1,&len,sizeof(len));
            len = val.GetSize();
            std::memcpy(buf + 1 + sizeof(len),&len,sizeof(len));
            this->offset_ += this->channel_->WriteAsync(buf,sizeof(buf),this->offset_);
            this->offset_ += this->channel_->WriteAsync(key,this->offset_);
            this->offset_ += this->channel_->WriteAsync(val,this->offset_);
        }

        void LogDelete(const sharpen::ByteBuffer &key)
        {
            if(!key.GetSize())
            {
                throw std::logic_error("empty key");
            }
            sharpen::Uint64 len = key.GetSize();
            char buf[1+sizeof(len)] = {0};
            buf[0] = sharpen::BinaryLoggerHelper::DeleteOp;
            std::memcpy(buf + 1,&len,sizeof(len));
            this->offset_ += this->channel_->WriteAsync(buf,sizeof(buf),this->offset_);
            this->offset_ += this->channel_->WriteAsync(key,this->offset_);
        }

        void ClearLogs()
        {
            this->offset_ = 0;
            this->channel_->Truncate();
        }

        void Restore(MapType &map)
        {
            sharpen::Uint64 offset{0},size{0};
            size = this->channel_->GetFileSize();
            sharpen::ByteBuffer buf{4096},key,value;
            char op;
            sharpen::BinaryLoggerHelper::RestoreStep step = sharpen::BinaryLoggerHelper::RestoreStep::ReadOp;
            sharpen::Size counter{0},lenBuf{0};
            while (offset != size)
            {
                sharpen::Size len = this->channel_->ReadAsync(buf,offset);
                this->RunStateMachine(buf.Data(),buf.Data() + len,step,op,key,value,lenBuf,counter,map);
                if(step == sharpen::BinaryLoggerHelper::RestoreStep::Error)
                {
                    //handle error
                    this->channel_->Truncate(offset);
                    this->offset_ = offset;
                    return;
                }
                offset += len;
            }
        }
    };   
}

#endif