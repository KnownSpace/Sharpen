#pragma once
#ifndef _SHARPEN_BYTEBUFFER_HPP
#define _SHARPEN_BYTEBUFFER_HPP

#include <functional>
#include <cassert>
#include <algorithm>

#include "ByteVector.hpp"
#include "ByteSlice.hpp"
#include "BufferOps.hpp"
#include "TypeTraits.hpp"
#include "PointerIterator.hpp"
#include "ReversePointerIterator.hpp"

namespace sharpen
{
    class ByteBuffer
    {
    private:
        using Vector = sharpen::ByteVector;

        using Self = sharpen::ByteBuffer;
        
        Vector vector_;
    public:
        using Iterator = typename Vector::Iterator;

        using ConstIterator = typename Vector::ConstIterator;

        using ReverseIterator = typename Vector::ReverseIterator;

        using ConstReverseIterator = typename Vector::ConstReverseIterator;

        ByteBuffer() = default;

        explicit ByteBuffer(std::size_t size);

        ByteBuffer(const char *p,std::size_t size);

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

        void PushBack(char val);

        std::size_t GetSize() const noexcept;

        void PopBack();

        char Front() const;

        char &Front();

        char Back() const;

        char &Back();

        char &Get(std::size_t index);

        char Get(std::size_t index) const;

        char GetOrDefault(std::size_t index,char defaultVal) const noexcept;

        char *Data() noexcept;

        const char* Data() const noexcept;

        inline char &operator[](std::size_t index)
        {
            return this->Get(index);
        }

        inline char operator[](std::size_t index) const
        {
            return this->Get(index);
        }

        void Reset() noexcept;

        void Extend(std::size_t size);

        void Extend(std::size_t size,char defaultValue);

        void ExtendTo(std::size_t size);

        void ExtendTo(std::size_t size,char defaultValue);

        void Append(const char *p,std::size_t size);

        void Append(const Self &other);

        template<typename _Iterator,typename _Checker = decltype(std::declval<Self>().PushBack(*std::declval<_Iterator>()))>
        void Append(_Iterator begin,_Iterator end)
        {
            this->vector_.Append(begin,end);
        }

        void Erase(std::size_t pos);

        void Erase(std::size_t begin,std::size_t end);

        void Erase(ConstIterator where);

        void Erase(ConstIterator begin,ConstIterator end);

        inline Iterator Begin() noexcept
        {
            return this->vector_.Begin();
        }

        inline ConstIterator Begin() const noexcept
        {
            return this->vector_.Begin();
        }

        inline ReverseIterator ReverseBegin() noexcept
        {
            return this->vector_.ReverseBegin();
        }

        inline ConstReverseIterator ReverseBegin() const noexcept
        {
            return this->vector_.ReverseBegin();
        }

        inline Iterator End() noexcept
        {
            return this->vector_.End();
        }

        inline ConstIterator End() const noexcept
        {
            return this->vector_.End();
        }

        inline ReverseIterator ReverseEnd() noexcept
        {
            return this->vector_.ReverseEnd();
        }

        inline ConstReverseIterator ReverseEnd() const noexcept
        {
            return this->vector_.ReverseEnd();
        }

        ConstIterator Find(char e) const noexcept;

        Iterator Find(char e) noexcept;

        ReverseIterator ReverseFind(char e) noexcept;

        ConstReverseIterator ReverseFind(char e) const noexcept;

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
            this->vector_.Clear();
        }

        inline std::uint32_t Adler32() const noexcept
        {
            return sharpen::Adler32(this->Data(),this->GetSize());
        }

        inline std::uint16_t Crc16() const noexcept
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

        inline std::int32_t CompareWith(const Self &other) const noexcept
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

        inline std::size_t Hash32() const noexcept
        {
            return sharpen::BufferHash32(this->Data(),this->GetSize());
        }

        inline std::uint64_t Hash64() const noexcept
        {
            return sharpen::BufferHash64(this->Data(),this->GetSize());
        }

        inline std::size_t Hash() const noexcept
        {
            return sharpen::BufferHash(this->Data(),this->GetSize());
        }

        inline bool Empty() const noexcept
        {
            return this->vector_.Empty();
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

        std::size_t ComputeSize() const noexcept;

        std::size_t LoadFrom(const char *data,std::size_t size);

        std::size_t LoadFrom(const sharpen::ByteBuffer &buf,std::size_t offset);

        inline std::size_t LoadFrom(const sharpen::ByteBuffer &buf)
        {
            return this->LoadFrom(buf,0);
        }

        std::size_t UnsafeStoreTo(char *data) const noexcept;

        std::size_t StoreTo(char *data,std::size_t size) const;

        std::size_t StoreTo(sharpen::ByteBuffer &buf,std::size_t offset) const;

        inline std::size_t StoreTo(sharpen::ByteBuffer &buf) const
        {
            return this->StoreTo(buf,0);
        }

        sharpen::ByteSlice GetSlice(std::size_t index,std::size_t size) const;

        sharpen::ByteSlice GetSlice(ConstIterator begin,ConstIterator end) const;
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