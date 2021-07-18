#pragma once
#ifndef _SHARPEN_HTTPBODY_HPP
#define _SHARPEN_HTTPBODY_HPP

#include <vector>
#include <type_traits>

#include "TypeDef.hpp"

namespace sharpen
{
    class ByteBuffer;

    class HttpBody
    {
    private:
        using Self = sharpen::HttpBody;
        using DataType = std::vector<char>;
        
        DataType data_;

        void CopyToMem(char *buf,sharpen::Size offset) const;
    public:
        using Iterator = typename DataType::iterator;

        using ConstIterator = typename DataType::const_iterator;

        using ReverseIterator = typename DataType::reverse_iterator;

        using ConstReverseIterator = typename DataType::const_reverse_iterator;

        HttpBody();

        explicit HttpBody(sharpen::Size size);

        HttpBody(const Self &other);

        HttpBody(Self &&other) noexcept;

        Self &operator=(const Self &other);

        Self &operator=(Self &&other) noexcept;

        ~HttpBody() noexcept = default;

        void Swap(Self &other) noexcept;

        inline void swap(Self &other) noexcept
        {
            this->Swap(other);
        }

        sharpen::Size CopyTo(char *buf,sharpen::Size size) const;

        sharpen::Size CopyTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const;

        inline sharpen::Size CopyTo(sharpen::ByteBuffer &buf) const
        {
            return this->CopyTo(buf,0);
        }

        void CopyFrom(const char *buf,sharpen::Size size);

        void CopyFrom(sharpen::ByteBuffer &buf,sharpen::Size offset,sharpen::Size size);

        void CopyFrom(sharpen::ByteBuffer &buf);

        sharpen::Size GetSize() const;

        Iterator Begin();

        ConstIterator Begin() const;

        ReverseIterator ReverseBegin();

        ConstReverseIterator ReverseBegin() const;

        Iterator End();

        ConstIterator End() const;

        ReverseIterator ReverseEnd();

        ConstReverseIterator ReverseEnd() const;

        void Realloc(sharpen::Size size);

        char *Data();
        
        const char *Data() const;

        char Get(sharpen::Size index) const;

        char &Get(sharpen::Size index);

        char operator[](sharpen::Size index) const
        {
            return this->Get(index);
        }
        
        char &operator[](sharpen::Size index)
        {
            return this->Get(index);
        }

        void Push(char c);

        void Append(const char *buf,sharpen::Size size);

        template<typename _Iterator,typename _Checker = decltype(std::declval<Self>().Push(*std::declval<_Iterator>()))>
        void Append(_Iterator begin,_Iterator end)
        {
            while (begin != end)
            {
                this->Push(*begin);
                begin++;
            }
        }

        void Erase(sharpen::Size pos);

        void Erase(sharpen::Size begin,sharpen::Size end);

        void Erase(ConstIterator where);

        void Erase(ConstIterator begin,ConstIterator end);

        void Clear();
    };
}

#endif