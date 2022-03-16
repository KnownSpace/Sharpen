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

#include "SstBlockHandle.hpp"

namespace sharpen
{
    class SstIndexBlock
    {
    public:
        using Comparator = sharpen::Int32(*)(const sharpen::ByteBuffer&,const sharpen::ByteBuffer&);
    private:
        using Self = SstIndexBlock;
        using DataBlockHandles = std::vector<sharpen::SstBlockHandle>;
        using Iterator = typename DataBlockHandles::iterator;
        using ConstIterator = typename DataBlockHandles::const_iterator;
    
        DataBlockHandles dataBlocks_;
        Comparator comp_;

        static bool Comp(const sharpen::SstBlockHandle &block,const sharpen::ByteBuffer &key) noexcept;

        static bool WarppedComp(Comparator comp,const sharpen::SstBlockHandle &block,const sharpen::ByteBuffer &key) noexcept;

        sharpen::Int32 CompKey(const sharpen::ByteBuffer &left,const sharpen::ByteBuffer &right) const noexcept;
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

        void LoadFrom(const char *data,sharpen::Size size);

        void LoadFrom(const sharpen::ByteBuffer &buf,sharpen::Size offset);

        inline void LoadFrom(const sharpen::ByteBuffer &buf)
        {
            this->LoadFrom(buf,0);
        }

        sharpen::Size ComputeNeedSize() const noexcept;

        sharpen::Size UnsafeStoreTo(char *data) const noexcept;

        sharpen::Size StoreTo(char *data,sharpen::Size size) const;

        sharpen::Size StoreTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const;

        inline sharpen::Size StoreTo(sharpen::ByteBuffer &buf) const
        {
            return this->StoreTo(buf,0);
        }

        Iterator Find(const sharpen::ByteBuffer &key) noexcept;

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

        inline sharpen::Size GetSize() const noexcept
        {
            return this->dataBlocks_.size();
        }

        inline void Reserve(sharpen::Size size)
        {
            this->dataBlocks_.reserve(this->GetSize() + size);
        }

        inline sharpen::SstBlockHandle &operator[](sharpen::Size index)
        {
            return this->dataBlocks_.at(index);
        }

        inline const sharpen::SstBlockHandle &operator[](sharpen::Size index) const
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