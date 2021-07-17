#pragma once
#ifndef _SHARPEN_BYTEBUFFER_HPP
#define _SHARPEN_BYTEBUFFER_HPP

#include <vector>
#include <algorithm>

#include "Noncopyable.hpp"
#include "TypeDef.hpp"

namespace sharpen
{
    class ByteBuffer:public sharpen::Noncopyable
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

        ByteBuffer(Self &&other) noexcept;

        virtual ~ByteBuffer() noexcept = default;

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

        void Reset(sharpen::Size size);

        void Extend(sharpen::Size size);

        void Extend(sharpen::Size size,sharpen::Char default);

        void Shrink();

        void Append(const sharpen::Char *p,sharpen::Size size);

        void Append(const Self &other);

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

        template<typename _Iterator>
        Iterator Search(const _Iterator begin,const _Iterator end)
        {
            return std::search(this->Begin(),this->End(),begin,end);
        }

        template<typename _Iterator>
        ConstIterator Search(const _Iterator begin,const _Iterator end) const
        {
            return std::search(this->Begin(),this->End(),begin,end);
        }
    };
} 

#endif