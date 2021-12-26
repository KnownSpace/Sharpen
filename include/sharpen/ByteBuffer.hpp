#pragma once
#ifndef _SHARPEN_BYTEBUFFER_HPP
#define _SHARPEN_BYTEBUFFER_HPP

#include <vector>
#include <algorithm>
#include <type_traits>

#include "Noncopyable.hpp"
#include "TypeDef.hpp"
#include "BufferOps.hpp"

namespace sharpen
{
    class ByteBuffer
    {
        using Vector = std::vector<sharpen::Char>;

        using Self = ByteBuffer;
        
    protected:
        Vector vector_;

        sharpen::Size mark_;

    private:
        void CheckAndMoveMark();

    public:
        using Iterator = typename Vector::iterator;

        using ConstIterator = typename Vector::const_iterator;

        using ReverseIterator = typename Vector::reverse_iterator;

        using ConstReverseIterator = typename Vector::const_reverse_iterator;

        ByteBuffer();

        explicit ByteBuffer(sharpen::Size size);

        explicit ByteBuffer(Vector &&vector) noexcept;

        ByteBuffer(const sharpen::Char *p,sharpen::Size size);

        ByteBuffer(const Self &other);

        ByteBuffer(Self &&other) noexcept;

        virtual ~ByteBuffer() noexcept = default;

        Self &operator=(const Self &other);

        Self &operator=(Self &&other) noexcept;

        //use by stl
        void swap(Self &other) noexcept;

        inline void Swap(Self &other) noexcept
        {
            this->swap(other);
        }

        void PushBack(sharpen::Char val);

        sharpen::Size GetSize() const;

        void PopBack();

        sharpen::Char Front() const;

        sharpen::Char &Front();

        sharpen::Char Back() const;

        sharpen::Char &Back();

        sharpen::Char &Get(sharpen::Size index);

        sharpen::Char Get(sharpen::Size index) const;

        sharpen::Char *Data();

        const sharpen::Char* Data() const;

        inline sharpen::Char &operator[](sharpen::Size index)
        {
            return this->Get(index);
        }

        inline sharpen::Char operator[](sharpen::Size index) const
        {
            return this->Get(index);
        }

        void Reserve(sharpen::Size size);

        void Reset();

        void Extend(sharpen::Size size);

        void Extend(sharpen::Size size,sharpen::Char defaultValue);

        void ExtendTo(sharpen::Size size);

        void ExtendTo(sharpen::Size size,sharpen::Char defaultValue);

        void Shrink();

        void Append(const sharpen::Char *p,sharpen::Size size);

        void Append(const Self &other);

        template<typename _Iterator,typename _Checker = decltype(std::declval<Self>().PushBack(*std::declval<_Iterator>()))>
        void Append(_Iterator begin,_Iterator end)
        {
            while (begin != end)
            {
                this->vector_.push_back(*begin);
                ++begin;
            }
        }

        void Erase(sharpen::Size pos);

        void Erase(sharpen::Size begin,sharpen::Size end);

        void Erase(ConstIterator where);

        void Erase(ConstIterator begin,ConstIterator end);

        void Mark(sharpen::Size pos);

        sharpen::Size Remaining() const;

        sharpen::Size GetMark() const;

        inline Iterator Begin()
        {
            return this->vector_.begin();
        }

        inline ConstIterator Begin() const
        {
            return this->vector_.cbegin();
        }

        inline ReverseIterator ReverseBegin()
        {
            return this->vector_.rbegin();
        }

        inline ConstReverseIterator ReverseBegin() const
        {
            return this->vector_.crbegin();
        }

        inline Iterator End()
        {
            return this->vector_.end();
        }

        inline ConstIterator End() const
        {
            return this->vector_.cend();
        }

        inline ReverseIterator ReverseEnd()
        {
            return this->vector_.rend();
        }

        inline ConstReverseIterator ReverseEnd() const
        {
            return this->vector_.crend();
        }

        ConstIterator Find(sharpen::Char e) const;

        Iterator Find(sharpen::Char e);

        ReverseIterator ReverseFind(sharpen::Char e);

        ConstReverseIterator ReverseFind(sharpen::Char e) const;

        template<typename _Iterator,typename _Check = decltype(std::declval<Self>().Get(0) == *std::declval<_Iterator>())>
        Iterator Search(const _Iterator begin,const _Iterator end)
        {
            return std::search(this->Begin(),this->End(),begin,end);
        }

        template<typename _Iterator,typename _Check = decltype(std::declval<Self>().Get(0) == *std::declval<_Iterator>())>
        ConstIterator Search(const _Iterator begin,const _Iterator end) const
        {
            return std::search(this->Begin(),this->End(),begin,end);
        }

        inline void Clear()
        {
            this->vector_.clear();
        }

        inline sharpen::Uint32 Adler32() const noexcept
        {
            return sharpen::Adler32(this->Data(),this->GetSize());
        }

        inline sharpen::Uint16 Crc16() const noexcept
        {
            return sharpen::Crc16(this->Data(),this->GetSize());
        }

        inline sharpen::ByteBuffer Base64Encode() const
        {
            sharpen::ByteBuffer buf{sharpen::ComputeBase64EncodeSize(this->GetSize())};
            bool success = sharpen::Base64Encode(buf.Data(),buf.GetSize(),this->Data(),this->GetSize());
            static_cast<void>(success);
            return buf;
        }

        inline sharpen::ByteBuffer Base64Decode() const
        {
            sharpen::ByteBuffer buf{sharpen::ComputeBase64DecodeSize(this->GetSize())};
            bool success = sharpen::Base64Decode(buf.Data(),buf.GetSize(),this->Data(),this->GetSize());
            static_cast<void>(success);
            return buf;
        }
    };
} 

#endif