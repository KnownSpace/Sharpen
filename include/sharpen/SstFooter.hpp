#pragma once
#ifndef _SHARPEN_SSTFOOTER_HPP
#define _SHARPEN_SSTFOOTER_HPP

/*
Footer Block
+----------------------------+
|Offset of Index Blocks      |  8 bytes
+----------------------------+
|Size of Index Blocks        |  8 bytes
+----------------------------+
|Offset of Meta Index Blocks |  8 bytes
+----------------------------+
|Size of Meta Index Blocks   |  8 bytes
+----------------------------+
*/

#include <utility>

#include "SstBlock.hpp"
#include "ByteBuffer.hpp"

namespace sharpen
{
    class SstFooter
    {
    private:
        using Self = sharpen::SstFooter;
    
        sharpen::SstBlock indexBlock_;
        sharpen::SstBlock metaIndexBlock_;
    public:
        SstFooter() noexcept = default;

        SstFooter(sharpen::SstBlock indexBlock,sharpen::SstBlock metaBlock)
            :indexBlock_(indexBlock)
            ,metaIndexBlock_(metaBlock)
        {}
    
        SstFooter(const Self &other) = default;
    
        SstFooter(Self &&other) noexcept = default;
    
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
                this->indexBlock_ = std::move(other.indexBlock_);
                this->metaIndexBlock_ = std::move(other.metaIndexBlock_);
            }
            return *this;
        }

        ~SstFooter() noexcept = default;

        const sharpen::SstBlock &IndexBlock() const noexcept
        {
            return this->indexBlock_;
        }

        sharpen::SstBlock &IndexBlock() noexcept
        {
            return this->indexBlock_;
        }

        const sharpen::SstBlock &MetaIndexBlock() const noexcept
        {
            return this->metaIndexBlock_;
        }

        sharpen::SstBlock &MetaIndexBlock() noexcept
        {
            return this->metaIndexBlock_;
        }

        void LoadFrom(const sharpen::ByteBuffer &buf,sharpen::Size offset);

        inline void LoadFrom(const sharpen::ByteBuffer &buf)
        {
            this->LoadFrom(buf,0);
        }

        void StoreTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const;

        inline void StoreTo(sharpen::ByteBuffer &buf) const
        {
            this->StoreTo(buf,0);
        }
    };
}

#endif