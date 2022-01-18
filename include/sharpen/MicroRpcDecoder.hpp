#pragma once
#ifndef _SHARPEN_MICRORPCDECODER_HPP
#define _SHARPEN_MICRORPCDECODER_HPP

#include "MicroRpcStack.hpp"
#include "Noncopyable.hpp"
#include "MicroRpcParseException.hpp"

namespace sharpen
{
    //TODO:complete the class
    class MicroRpcDecoder:public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::MicroRpcDecoder;

        enum class Step
        {
            WaitMetadata = 0,
            WaitSize = 1,
            WaitData = 2,
            Completed = 3
        };

        sharpen::MicroRpcStack *stack_;
        sharpen::Size ite_;
        Step step_;
        sharpen::Size typeSize_;
        sharpen::Size record_;
        bool completed_;

        const char *RunStateMachine(const char *begin,const char *end);
    public:
        MicroRpcDecoder() noexcept;

        MicroRpcDecoder(Self &&other) noexcept;

        Self &operator=(Self &&other) noexcept;

        ~MicroRpcDecoder() noexcept = default;

        sharpen::Size Decode(const char *data,sharpen::Size size);

        inline sharpen::Size Decode(const sharpen::ByteBuffer &buf,sharpen::Size offset)
        {
            return this->Decode(buf.Data() + offset,buf.GetSize() - offset);
        }

        inline sharpen::Size Decode(const sharpen::ByteBuffer &buf)
        {
            return this->Decode(buf,0);
        }

        bool IsCompleted() const noexcept
        {
            return this->completed_;
        }

        void SetCompleted(bool completed) noexcept
        {
            this->completed_ = completed;
        }

        void Bind(sharpen::MicroRpcStack &stack) noexcept
        {
            this->stack_ = &stack;
        }
    };
}

#endif