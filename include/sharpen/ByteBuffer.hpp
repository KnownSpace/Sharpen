#pragma once
#ifndef _SHARPEN_BYTEBUFFER_HPP
#define _SHARPEN_BYTEBUFFER_HPP

#include <vector>
#include <algorithm>
#include <functional>
#include <cassert>

#include "TypeDef.hpp"
#include "BufferOps.hpp"
#include "TypeTraits.hpp"

namespace sharpen
{
    class ByteBuffer
    {
    private:
        using Vector = std::vector<sharpen::Char>;

        using Self = ByteBuffer;
        
        Vector vector_;
    public:
        using Iterator = typename Vector::iterator;

        using ConstIterator = typename Vector::const_iterator;

        using ReverseIterator = typename Vector::reverse_iterator;

        using ConstReverseIterator = typename Vector::const_reverse_iterator;

        ByteBuffer() = default;

        explicit ByteBuffer(sharpen::Size size);

        explicit ByteBuffer(Vector vector) noexcept;

        ByteBuffer(const sharpen::Char *p,sharpen::Size size);

        ByteBuffer(const Self &other) = default;

        ByteBuffer(Self &&other) noexcept = default;

        ~ByteBuffer() noexcept = default;

        inline Self &operator=(const Self &other)
        {
            if(this != std::addressof(other))
            {
                sharpen::ByteBuffer tmp{other};
                std::swap(tmp,*this);
            }
            return *this;
        }

        inline Self &operator=(Self &&other) noexcept
        {
            if(this != std::addressof(other))
            {
                this->vector_ = std::move(other.vector_);
            }
            return *this;
        }

        void PushBack(sharpen::Char val);

        sharpen::Size GetSize() const noexcept;

        void PopBack();

        sharpen::Char Front() const;

        sharpen::Char &Front();

        sharpen::Char Back() const;

        sharpen::Char &Back();

        sharpen::Char &Get(sharpen::Size index);

        sharpen::Char Get(sharpen::Size index) const;

        sharpen::Char GetOrDefault(sharpen::Size index,sharpen::Char defaultVal) const noexcept;

        sharpen::Char *Data() noexcept;

        const sharpen::Char* Data() const noexcept;

        inline sharpen::Char &operator[](sharpen::Size index)
        {
            return this->Get(index);
        }

        inline sharpen::Char operator[](sharpen::Size index) const
        {
            return this->Get(index);
        }

        void Reserve(sharpen::Size size);

        void Reset() noexcept;

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

        inline Iterator Begin() noexcept
        {
            return this->vector_.begin();
        }

        inline ConstIterator Begin() const noexcept
        {
            return this->vector_.cbegin();
        }

        inline ReverseIterator ReverseBegin() noexcept
        {
            return this->vector_.rbegin();
        }

        inline ConstReverseIterator ReverseBegin() const noexcept
        {
            return this->vector_.crbegin();
        }

        inline Iterator End() noexcept
        {
            return this->vector_.end();
        }

        inline ConstIterator End() const noexcept
        {
            return this->vector_.cend();
        }

        inline ReverseIterator ReverseEnd() noexcept
        {
            return this->vector_.rend();
        }

        inline ConstReverseIterator ReverseEnd() const noexcept
        {
            return this->vector_.crend();
        }

        ConstIterator Find(sharpen::Char e) const noexcept;

        Iterator Find(sharpen::Char e) noexcept;

        ReverseIterator ReverseFind(sharpen::Char e) noexcept;

        ConstReverseIterator ReverseFind(sharpen::Char e) const noexcept;

        template<typename _Iterator,typename _Check = decltype(std::declval<Self>().Get(0) == *std::declval<_Iterator>())>
        inline Iterator Search(const _Iterator begin,const _Iterator end)
        {
            return std::search(this->Begin(),this->End(),begin,end);
        }

        template<typename _Iterator,typename _Check = decltype(std::declval<Self>().Get(0) == *std::declval<_Iterator>())>
        inline ConstIterator Search(const _Iterator begin,const _Iterator end) const
        {
            return std::search(this->Begin(),this->End(),begin,end);
        }

        inline void Clear() noexcept
        {
            this->vector_.clear();
        }

        inline void ClearAndShrink() noexcept
        {
            Vector tmp;
            std::swap(this->vector_,tmp);
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

        inline sharpen::Int32 CompareWith(const Self &other) const noexcept
        {
            return sharpen::BufferCompare(this->Data(),this->GetSize(),other.Data(),other.GetSize());
        }

        inline bool operator>(const Self &other) const noexcept
        {
            return this->CompareWith(other) > 0;
        }

        inline bool operator<(const Self &other) const noexcept
        {
            return this->CompareWith(other) < 0;
        }

        inline bool operator>=(const Self &other) const noexcept
        {
            return this->CompareWith(other) >= 0;
        }

        inline bool operator<=(const Self &other) const noexcept
        {
            return this->CompareWith(other) <= 0;
        }

        inline bool operator==(const Self &other) const noexcept
        {
            return this->CompareWith(other) == 0;
        }

        inline bool operator!=(const Self &other) const noexcept
        {
            return this->CompareWith(other) != 0;
        }

        inline sharpen::Size Hash32() const noexcept
        {
            return sharpen::BufferHash32(this->Data(),this->GetSize());
        }

        inline sharpen::Uint64 Hash64() const noexcept
        {
            return sharpen::BufferHash64(this->Data(),this->GetSize());
        }

        inline sharpen::Size Hash() const noexcept
        {
            return sharpen::BufferHash(this->Data(),this->GetSize());
        }

        inline bool Empty() const noexcept
        {
            return this->vector_.empty();
        }

        template<typename _T,typename _Check = sharpen::EnableIf<std::is_standard_layout<_T>::value>>
        _T &As() noexcept
        {
            assert(this->GetSize() == sizeof(_T));
            return *reinterpret_cast<_T*>(this->Data());
        }

        template<typename _T,typename _Check = sharpen::EnableIf<std::is_standard_layout<_T>::value>>
        const _T &As() const noexcept
        {
            assert(this->GetSize() == sizeof(_T));
            return *reinterpret_cast<const _T*>(this->Data());
        }

        sharpen::Size ComputeSize() const noexcept;

        sharpen::Size LoadFrom(const char *data,sharpen::Size size);

        sharpen::Size LoadFrom(const sharpen::ByteBuffer &buf,sharpen::Size offset);

        inline sharpen::Size LoadFrom(const sharpen::ByteBuffer &buf)
        {
            return this->LoadFrom(buf,0);
        }

        sharpen::Size UnsafeStoreTo(char *data) const noexcept;

        sharpen::Size StoreTo(char *data,sharpen::Size size) const;

        sharpen::Size StoreTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const;

        inline sharpen::Size StoreTo(sharpen::ByteBuffer &buf) const
        {
            return this->StoreTo(buf,0);
        }
    };
} 

namespace std
{
    template<>
    struct hash<sharpen::ByteBuffer>
    {
        inline std::size_t operator()(const sharpen::ByteBuffer& buf) const noexcept
        {
            return buf.Hash();
        }
    };
}

#endif