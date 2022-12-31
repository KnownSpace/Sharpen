#pragma once
#ifndef _SHARPEN_IPERSISTENTMAP_HPP
#define _SHARPEN_IPERSISTENTMAP_HPP

#include <cassert>

#include "ByteBuffer.hpp"
#include "Optional.hpp"

namespace sharpen
{
    class IStatusMap
    {
    private:
        using Self = sharpen::IStatusMap;
    protected:

        virtual sharpen::Optional<sharpen::ByteBuffer> NviLookup(const sharpen::ByteBuffer &key) const = 0;

        virtual void NviWrite(sharpen::ByteBuffer key,sharpen::ByteBuffer value) = 0;
    public:
    
        IStatusMap() noexcept = default;
    
        IStatusMap(const Self &other) noexcept = default;
    
        IStatusMap(Self &&other) noexcept = default;
    
        Self &operator=(const Self &other) noexcept = default;
    
        Self &operator=(Self &&other) noexcept = default;
    
        virtual ~IStatusMap() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline void Write(sharpen::ByteBuffer key,sharpen::ByteBuffer value)
        {
            assert(!key.Empty());
            return this->NviWrite(std::move(key),std::move(value));
        }

        inline sharpen::Optional<sharpen::ByteBuffer> Lookup(const sharpen::ByteBuffer &key) const
        {
            assert(!key.Empty());
            return this->NviLookup(key);
        }
    };
}

#endif