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

#include "FilePointer.hpp"
#include "ByteBuffer.hpp"

namespace sharpen
{
    class SstFooter
    {
    private:
        using Self = sharpen::SstFooter;
    
        sharpen::FilePointer indexBlock_;
        sharpen::FilePointer metaIndexBlock_;

    public:
        SstFooter() noexcept = default;

        SstFooter(sharpen::FilePointer indexBlock,sharpen::FilePointer metaBlock)
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

        const sharpen::FilePointer &IndexBlock() const noexcept
        {
            return this->indexBlock_;
        }

        sharpen::FilePointer &IndexBlock() noexcept
        {
            return this->indexBlock_;
        }

        const sharpen::FilePointer &MetaIndexBlock() const noexcept
        {
            return this->metaIndexBlock_;
        }

        sharpen::FilePointer &MetaIndexBlock() noexcept
        {
            return this->metaIndexBlock_;
        }

        void LoadFrom(const char *data,std::size_t size);

        void LoadFrom(const sharpen::ByteBuffer &buf,std::size_t offset);

        inline void LoadFrom(const sharpen::ByteBuffer &buf)
        {
            this->LoadFrom(buf,0);
        }

        void UnsafeStoreTo(char *data) const noexcept;

        void StoreTo(char *data,std::size_t size) const;

        void StoreTo(sharpen::ByteBuffer &buf,std::size_t offset) const;

        inline void StoreTo(sharpen::ByteBuffer &buf) const
        {
            this->StoreTo(buf,0);
        }
    };
}

#endif