#pragma once
#ifndef _SHARPEN_SSTINDEXBLOCK_HPP
#define _SHARPEN_SSTINDEXBLOCK_HPP

/*
Index Block
+---------------------+
|Data Block1 Key Size | 8 bytes
+---------------------+
|Data Block1 Key      | Index Key
+---------------------+
|Offset of Data Block1| 8 bytes
+---------------------+
|Size of Data Block1  | 8 bytes
+---------------------+
|        .....        |
+---------------------+
|Data BlockN Key Size | 8 bytes
+---------------------+
|Data BlockN Key      |
+---------------------+
|Offset of Data BlockN| 8 bytes
+---------------------+
|Size of Data BlockN  | 8 bytes
+---------------------+
*/

#include <vector>
#include <algorithm>

#include "SstBlockHandle.hpp"

namespace sharpen
{
    class SstIndexBlock
    {
    private:
        using Self = SstIndexBlock;
        using DataBlockHandles = std::vector<sharpen::SstBlockHandle>;
        using Iterator = typename DataBlockHandles::iterator;
        using ConstIterator = typename DataBlockHandles::const_iterator;
    
        DataBlockHandles dataBlocks_;

        Iterator Find(const sharpen::ByteBuffer &key) noexcept;

        void InternalStoreTo(char *data) const noexcept;
    public:
    
        SstIndexBlock() noexcept = default;

        explicit SstIndexBlock(DataBlockHandles blocks) noexcept
            :dataBlocks_(std::move(blocks))
        {}
    
        SstIndexBlock(const Self &other) = default;
    
        SstIndexBlock(Self &&other) noexcept = default;
    
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
                this->dataBlocks_ = std::move(other.dataBlocks_);
            }
            return *this;
        }
    
        ~SstIndexBlock() noexcept = default;

        const DataBlockHandles &Blocks() const noexcept
        {
            return this->dataBlocks_;
        }

        void LoadFrom(const char *data,sharpen::Size size);

        void LoadFrom(const sharpen::ByteBuffer &buf,sharpen::Size size,sharpen::Size offset);

        inline void LoadFrom(const sharpen::ByteBuffer &buf,sharpen::Size size)
        {
            this->LoadFrom(buf,size,0);
        }

        sharpen::Size ComputeNeedSize() const noexcept;

        sharpen::Size StoreTo(char *data,sharpen::Size size) const;

        sharpen::Size StoreTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const;

        inline sharpen::Size StoreTo(sharpen::ByteBuffer &buf) const
        {
            return this->StoreTo(buf,0);
        }

        ConstIterator Find(const sharpen::ByteBuffer &key) const noexcept;

        void Sort() noexcept
        {
            std::sort(this->dataBlocks_.begin(),this->dataBlocks_.end());
        }

        inline Iterator Begin()
        {
            return this->dataBlocks_.begin();
        }

        inline ConstIterator Begin() const
        {
            return this->dataBlocks_.cbegin();
        }

        inline Iterator End()
        {
            return this->dataBlocks_.end();
        }

        inline ConstIterator End() const
        {
            return this->dataBlocks_.cend();
        }

        void Put(sharpen::SstBlockHandle block);

        void Put(sharpen::ByteBuffer key,const sharpen::SstBlock &block);

        void Delete(const sharpen::ByteBuffer &key) noexcept;

        void Update(const sharpen::ByteBuffer &oldKey,sharpen::SstBlockHandle block);

        void Update(const sharpen::ByteBuffer &oldKey,sharpen::ByteBuffer newKey,const sharpen::SstBlock &block);

        inline void Clear() noexcept
        {
            this->dataBlocks_.clear();
        }
    };
}

#endif