#pragma once
#ifndef _SHARPEN_SIMPLEPERSISTENTMAP_HPP
#define _SHARPEN_SIMPLEPERSISTENTMAP_HPP

#include <map>

#include "IFileChannel.hpp"
#include "IPersistentMap.hpp"
#include "AsyncRwLock.hpp"

namespace sharpen
{
    class SimplePersistentMap:public sharpen::IPersistentMap,public sharpen::Noncopyable,public sharpen::Nonmovable
    {
    private:
        using Self = sharpen::SimplePersistentMap;
    
        sharpen::FileChannelPtr file_;
        std::map<sharpen::ByteBuffer,sharpen::ByteBuffer> map_;
        mutable sharpen::AsyncRwLock lock_;
        
        void ReadFromFile(std::uint64_t offset);

        void SaveFile();

        virtual sharpen::Optional<sharpen::ByteBuffer> NviLookup(const sharpen::ByteBuffer &key) const override;

        virtual void NviWrite(sharpen::ByteBuffer key,sharpen::ByteBuffer value) override;
    public:
    
        explicit SimplePersistentMap(sharpen::FileChannelPtr file);
    
        virtual ~SimplePersistentMap() noexcept = default;
    
        inline const Self &Const() const noexcept
        {
            return *this;
        }
    };
}

#endif