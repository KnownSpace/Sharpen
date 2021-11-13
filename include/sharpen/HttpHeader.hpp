#pragma once
#ifndef _SHARPEN_HTTPHEADER_HPP
#define _SHARPEN_HTTPHEADER_HPP

#include <unordered_map>
#include <string>

#include "TypeDef.hpp"

namespace sharpen
{
    class ByteBuffer;

    class HttpHeader
    {
    private:
        using Self = sharpen::HttpHeader;
        using Headers = std::unordered_map<std::string,std::string>;
        using Iterator = typename Headers::iterator;
        using ConstIterator = typename Headers::const_iterator;

        Headers headers_;

        void CopyToMem(char *buf,sharpen::Size offset) const;
    public:
        HttpHeader();

        HttpHeader(const Self &other);

        HttpHeader(Self &&other) noexcept;

        ~HttpHeader() noexcept = default;

        Self &operator=(const Self &other);

        Self &operator=(Self &&other) noexcept;

        void AddHeader(std::string field,std::string value);

        const std::string &GetHeader(const std::string &field) const;

        std::string &GetHeader(const std::string &field);

        void RemoveHeader(const std::string &field);

        const std::string &operator[](const std::string &field) const
        {
            return this->GetHeader(field);
        }

        std::string &operator[](const std::string &field)
        {
            return this->GetHeader(field);
        }

        bool ExistHeader(const std::string &field) const;

        sharpen::Size CopyTo(char *buf,sharpen::Size size) const;

        sharpen::Size CopyTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const;

        inline sharpen::Size CopyTo(sharpen::ByteBuffer &buf) const
        {
            return this->CopyTo(buf,0);
        }

        sharpen::Size ComputeSize() const;

        void Swap(Self &other) noexcept;

        void swap(Self &other) noexcept
        {
            this->Swap(other);
        }

        void Clear();

        Iterator Begin();

        Iterator End();

        ConstIterator Begin() const;

        ConstIterator End() const;
    };
}

#endif