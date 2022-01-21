#pragma once
#ifndef _SHARPEN_SSTDATABLOCK_HPP
#define _SHARPEN_SSTDATABLOCK_HPP

/*
Key Value Pair
+---------------+
| SharedKeySize | varint
+---------------+
| UniqueKeySize | varint
+---------------+
| UniqueKey     |
+---------------+
| ValueSize     | varint 
+---------------+
| Value         |
+---------------+

Key Value Group
+-----------------+
| Key Value Pair1 |
+-----------------+
|       ...       |
+-----------------+
| Key Value PairN |
+-----------------+

DataBlock                   about 4*1024 bytes
+-------------------------+
| Crc16                   | 2 bytes
+-------------------------+
| Number of Group         | varint
+-------------------------+
| Key Value Group Offset1 | relative offset - 4 bytes
+-------------------------+
|          ...            |
+-------------------------+
| Key Value Group OffsetN |
+-------------------------+
| Key Value Group1        |
+-------------------------+
|          ...            |
+-------------------------+
| Key Value GroupN        |
+-------------------------+
*/

#include <vector>

#include "SstBlockHandle.hpp"
#include "SstKeyValueGroup.hpp"
#include "DataCorruptionException.hpp"

namespace sharpen
{
    class SstDataBlock
    {
    private:
        using Self = SstDataBlock;
        using Groups = std::vector<sharpen::SstKeyValueGroup>;
        using Iterator = typename Groups::iterator;
        using ConstIterator = typename Groups::const_iterator;

        static constexpr sharpen::Size maxKeyPerGroups_{16};

        Groups groups_;

        static bool Comp(const sharpen::SstKeyValueGroup &group,const sharpen::ByteBuffer &key) noexcept;
    public:
    
        SstDataBlock() = default;
    
        SstDataBlock(const Self &other) = default;
    
        SstDataBlock(Self &&other) noexcept = default;
    
        inline Self &operator=(const Self &other)
        {
            Self tmp{other};
            std::swap(tmp,*this);
            return *this;
        }
    
        Self &operator=(Self &&other) noexcept = default;
    
        ~SstDataBlock() noexcept = default;

        void LoadFrom(const char *data,sharpen::Size size);

        void LoadFrom(const sharpen::ByteBuffer &buf,sharpen::Size offset);

        inline void LoadFrom(const sharpen::ByteBuffer &buf)
        {
            this->LoadFrom(buf,0);
        }

        sharpen::Size ComputeSize() const noexcept;

        sharpen::Size UnsafeStoreTo(char *data) const;

        sharpen::Size StoreTo(char *data,sharpen::Size size) const;

        sharpen::Size StoreTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const;

        inline sharpen::Size StoreTo(sharpen::ByteBuffer &buf) const
        {
            return this->StoreTo(buf,0);
        }

        ConstIterator FindGroup(const sharpen::ByteBuffer &key) const;

        inline ConstIterator Begin() const noexcept
        {
            return this->groups_.cbegin();
        }

        inline ConstIterator End() const noexcept
        {
            return this->groups_.cend();
        }

        inline bool Empty() const noexcept
        {
            return this->groups_.empty();
        }

        inline sharpen::Size GetSize() const noexcept
        {
            return this->groups_.size();
        }

        bool Exist(const sharpen::ByteBuffer &key) const;

        void Put(sharpen::ByteBuffer key,sharpen::ByteBuffer value);

        void Delete(const sharpen::ByteBuffer &key);

        Iterator FindGroup(const sharpen::ByteBuffer &key);

        sharpen::Size ComputeKeyCount() const noexcept;
    };
}

#endif