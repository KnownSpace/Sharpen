#pragma once
#ifndef _SHARPEN_BINARYSERIALIZABLE_HPP
#define _SHARPEN_BINARYSERIALIZABLE_HPP

#include "BinarySerializator.hpp"

namespace sharpen
{   
    template<typename _Object>
    class BinarySerializable
    {
    private:
        using Self = sharpen::BinarySerializable<_Object>;
        
    protected:
        using Serializator = sharpen::BinarySerializator;
        using Helper = Self::Serializator;
    public:
    
        BinarySerializable() noexcept = default;
    
        BinarySerializable(const Self &other) noexcept = default;
    
        BinarySerializable(Self &&other) noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    
        ~BinarySerializable() noexcept = default;

        sharpen::Size ComputeSize() const noexcept
        {
            return Serializator::ComputeSize(*static_cast<const _Object*>(this));
        }

        sharpen::Size LoadFrom(const char *data,sharpen::Size size)
        {
            return Serializator::LoadFrom(*static_cast<_Object*>(this),data,size);
        }

        sharpen::Size LoadFrom(const sharpen::ByteBuffer &buf,sharpen::Size offset)
        {
            return Serializator::LoadFrom(*static_cast<_Object*>(this),buf,offset);
        }

        sharpen::Size LoadFrom(const sharpen::ByteBuffer &buf)
        {
            return Serializator::LoadFrom(*static_cast<_Object*>(this),buf,0);
        }

        sharpen::Size UnsafeStoreTo(char *data) const noexcept
        {
            return Serializator::UnsafeStoreTo(*static_cast<_Object*>(this),data);
        }

        sharpen::Size StoreTo(char *data,sharpen::Size size) const
        {
            return Serializator::StoreTo(*static_cast<const _Object*>(this),data,size);
        }

        sharpen::Size StoreTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const
        {
            return Serializator::StoreTo(*static_cast<const _Object*>(this),buf,offset);
        }

        sharpen::Size StoreTo(sharpen::ByteBuffer &buf) const
        {
            return Serializator::StoreTo(*static_cast<const _Object*>(this),buf);
        }

        const sharpen::BinarySerializable<_Object> &Serialize() const noexcept
        {
            return *this;
        }

        sharpen::BinarySerializable<_Object> &Unserialize() noexcept
        {
            return *this;
        }
    };   
}

#endif