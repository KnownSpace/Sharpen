#pragma once
#ifndef _SHARPEN_SSTBLOCKHANDLE_HPP
#define _SHARPEN_SSTBLOCKHANDLE_HPP

/*
+---------+
|Key Size | 8 bytes
+---------+
|Key      |
+---------+
|Offset   | 8 bytes
+---------+
|Size     | 8 bytes
+---------+
*/

#include "SstBlock.hpp"
#include "ByteBuffer.hpp"
#include "IFileChannel.hpp"

namespace sharpen
{
    class SstBlockHandle
    {
    private:
        using Self = sharpen::SstBlockHandle;
    
        sharpen::ByteBuffer key_;
        sharpen::SstBlock block_;
    public:
    
        SstBlockHandle(sharpen::ByteBuffer key,const sharpen::SstBlock &block) noexcept
            :key_(std::move(key))
            ,block_(block)
        {}
    
        SstBlockHandle(const Self &other) = default;
    
        SstBlockHandle(Self &&other) noexcept = default;
    
        inline Self &operator=(const Self &other)
        {
            Self tmp{other};
            std::swap(tmp,*this);
            return *this;
        }
    
        Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->key_ = std::move(other.key_);
                this->block_ = std::move(other.block_);
            }
            return *this;
        }
    
        ~SstBlockHandle() noexcept = default;

        const sharpen::ByteBuffer &Key() const noexcept
        {
            return this->key_;
        }

        const sharpen::SstBlock &Block() const noexcept
        {
            return this->block_;
        }
    };
}

#endif