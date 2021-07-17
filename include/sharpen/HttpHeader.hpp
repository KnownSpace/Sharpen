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

        void CopyTo(char *buf,sharpen::Size size) const;

        void CopyTo(sharpen::ByteBuffer &buf) const;

        void CopyTo(sharpen::ByteBuffer &buf,sharpen::Size offset) const;

        sharpen::Size ComputeSize() const;

        void Swap(Self &other) noexcept;

        void swap(Self &other) noexcept
        {
            this->Swap(other);
        }
    };
}

#endif