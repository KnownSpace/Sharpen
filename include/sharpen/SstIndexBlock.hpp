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
    public:
        using Comparator = std::int32_t(*)(const sharpen::ByteBuffer&,const sharpen::ByteBuffer&);
    private:
        using Self = SstIndexBlock;
        using DataBlockHandles = std::vector<sharpen::SstBlockHandle>;
    public:
        
        using Iterator = typename DataBlockHandles::iterator;
        using ConstIterator = typename DataBlockHandles::const_iterator;
        using ReverseIterator = typename DataBlockHandles::reverse_iterator;
        using ConstReverseIterator = typename DataBlockHandles::const_reverse_iterator;
    private:
    
        DataBlockHandles dataBlocks_;
        Comparator comp_;

        static bool Comp(const sharpen::SstBlockHandle &block,const sharpen::ByteBuffer &key) noexcept;

        static bool WarppedComp(Comparator comp,const sharpen::SstBlockHandle &block,const sharpen::ByteBuffer &key) noexcept;

        std::int32_t CompKey(const sharpen::ByteBuffer &left,const sharpen::ByteBuffer &right) const noexcept;
    public:
    
        SstIndexBlock();

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
                this->comp_ = other.comp_;
                other.comp_ = nullptr;
            }
            return *this;
        }
    
        ~SstIndexBlock() noexcept = default;

        const DataBlockHandles &Blocks() const noexcept
        {
            return this->dataBlocks_;
        }

        void LoadFrom(const char *data,std::size_t size);

        void LoadFrom(const sharpen::ByteBuffer &buf,std::size_t offset);

        inline void LoadFrom(const sharpen::ByteBuffer &buf)
        {
            this->LoadFrom(buf,0);
        }

        std::size_t ComputeNeedSize() const noexcept;

        std::size_t UnsafeStoreTo(char *data) const noexcept;

        std::size_t StoreTo(char *data,std::size_t size) const;

        std::size_t StoreTo(sharpen::ByteBuffer &buf,std::size_t offset) const;

        inline std::size_t StoreTo(sharpen::ByteBuffer &buf) const
        {
            return this->StoreTo(buf,0);
        }

        Iterator Find(const sharpen::ByteBuffer &key) noexcept;

        ConstIterator Find(const sharpen::ByteBuffer &key) const noexcept;

        void Sort() noexcept
        {
            std::sort(this->dataBlocks_.begin(),this->dataBlocks_.end());
        }

        inline Iterator Begin() noexcept
        {
            return this->dataBlocks_.begin();
        }

        inline ConstIterator Begin() const noexcept
        {
            return this->dataBlocks_.cbegin();
        }

        inline Iterator End() noexcept
        {
            return this->dataBlocks_.end();
        }

        inline ConstIterator End() const noexcept
        {
            return this->dataBlocks_.cend();
        }

        inline ReverseIterator ReverseBegin() noexcept
        {
            return this->dataBlocks_.rbegin();
        }

        inline ConstReverseIterator ReverseBegin() const noexcept
        {
            return this->dataBlocks_.rbegin();
        }

        inline ReverseIterator ReverseEnd() noexcept
        {
            return this->dataBlocks_.rend();
        }

        inline ConstReverseIterator ReverseEnd() const noexcept
        {
            return this->dataBlocks_.rend();
        }

        void Put(sharpen::SstBlockHandle block);

        void Put(sharpen::ByteBuffer key,const sharpen::FilePointer &block);

        void Delete(const sharpen::ByteBuffer &key) noexcept;

        inline void Clear() noexcept
        {
            this->dataBlocks_.clear();
        }

        inline bool Empty() const noexcept
        {
            return this->dataBlocks_.empty();
        }

        inline std::size_t GetSize() const noexcept
        {
            return this->dataBlocks_.size();
        }

        inline void Reserve(std::size_t size)
        {
            this->dataBlocks_.reserve(this->GetSize() + size);
        }

        inline sharpen::SstBlockHandle &operator[](std::size_t index)
        {
            return this->dataBlocks_.at(index);
        }

        inline const sharpen::SstBlockHandle &operator[](std::size_t index) const
        {
            return this->dataBlocks_.at(index);
        }

        inline Comparator GetComparator() const noexcept
        {
            return this->comp_;
        }

        inline void SetComparator(Comparator comp) noexcept
        {
            this->comp_ = comp;
        }
    };
}

#endif