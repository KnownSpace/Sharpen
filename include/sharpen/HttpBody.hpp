#pragma once
#ifndef _SHARPEN_HTTPBODY_HPP
#define _SHARPEN_HTTPBODY_HPP

#include <vector>
#include <type_traits>

#include <cstdint>
#include <cstddef>

namespace sharpen
{
    class ByteBuffer;

    class HttpBody
    {
    private:
        using Self = sharpen::HttpBody;
        using DataType = std::vector<char>;
        
        DataType data_;

        void CopyToMem(char *buf,std::size_t offset) const;
    public:
        using Iterator = typename DataType::iterator;

        using ConstIterator = typename DataType::const_iterator;

        using ReverseIterator = typename DataType::reverse_iterator;

        using ConstReverseIterator = typename DataType::const_reverse_iterator;

        HttpBody();

        explicit HttpBody(std::size_t size);

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

        std::size_t CopyTo(char *buf,std::size_t size) const;

        std::size_t CopyTo(sharpen::ByteBuffer &buf,std::size_t offset) const;

        inline std::size_t CopyTo(sharpen::ByteBuffer &buf) const
        {
            return this->CopyTo(buf,0);
        }

        void CopyFrom(const char *buf,std::size_t size);

        void CopyFrom(sharpen::ByteBuffer &buf,std::size_t offset,std::size_t size);

        void CopyFrom(sharpen::ByteBuffer &buf);

        std::size_t GetSize() const;

        Iterator Begin();

        ConstIterator Begin() const;

        ReverseIterator ReverseBegin();

        ConstReverseIterator ReverseBegin() const;

        Iterator End();

        ConstIterator End() const;

        ReverseIterator ReverseEnd();

        ConstReverseIterator ReverseEnd() const;

        void Realloc(std::size_t size);

        char *Data();
        
        const char *Data() const;

        char Get(std::size_t index) const;

        char &Get(std::size_t index);

        char operator[](std::size_t index) const
        {
            return this->Get(index);
        }
        
        char &operator[](std::size_t index)
        {
            return this->Get(index);
        }

        void Push(char c);

        void Append(const char *buf,std::size_t size);

        template<typename _Iterator,typename _Checker = decltype(std::declval<Self>().Push(*std::declval<_Iterator>()))>
        void Append(_Iterator begin,_Iterator end)
        {
            while (begin != end)
            {
                this->Push(*begin);
                begin++;
            }
        }

        void Erase(std::size_t pos);

        void Erase(std::size_t begin,std::size_t end);

        void Erase(ConstIterator where);

        void Erase(ConstIterator begin,ConstIterator end);

        void Clear();
    };
}

#endif