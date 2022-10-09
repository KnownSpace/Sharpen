#pragma once
#ifndef _SHARPEN_IREMOTEPROCEDURE_HPP
#define _SHARPEN_IREMOTEPROCEDURE_HPP

#include "ByteBuffer.hpp"

namespace sharpen
{
    class IRemoteProcedure
    {
    private:
        using Self = sharpen::IRemoteProcedure;
    protected:
    public:
    
        IRemoteProcedure() noexcept = default;
    
        IRemoteProcedure(const Self &other) noexcept = default;
    
        IRemoteProcedure(Self &&other) noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    
        virtual ~IRemoteProcedure() noexcept = default;

        virtual sharpen::ByteBuffer Invoke(const char *message,std::size_t size) = 0;

        inline sharpen::ByteBuffer Invoke(const sharpen::ByteBuffer &message,std::size_t offset)
        {
            assert(message.GetSize() >= offset);
            return this->Invoke(message.Data() + offset,message.GetSize() - offset);
        }

        inline sharpen::ByteBuffer Invoke(const sharpen::ByteBuffer &message)
        {
            return this->Invoke(message,0);
        }
    };
}

#endif