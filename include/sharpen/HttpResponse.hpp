#pragma once
#ifndef _SHARPEN_HTTPRESPONSE_HPP
#define _SHARPEN_HTTPRESPONSE_HPP

#include "HttpVersion.hpp"
#include "HttpStatusCode.hpp"
#include "HttpBody.hpp"
#include "HttpHeader.hpp"

namespace sharpen
{
    class ByteBuffer;

    class HttpParser;

    class HttpResponseDecoder;

    class HttpResponse
    {
    private:
        using Self = sharpen::HttpResponse;
        //VERSION STATUSCODE STATUSNAME\r\n
        //field: value\r\n
        //...
        //field: value\r\n\r\n
        //body

        sharpen::HttpVersion version_;

        sharpen::HttpStatusCode status_;

        sharpen::HttpHeader header_;

        sharpen::HttpBody body_;
    public:
        HttpResponse();

        explicit HttpResponse(sharpen::HttpVersion version,sharpen::HttpStatusCode status);

        HttpResponse(const Self &other);

        HttpResponse(Self &&other) noexcept;

        Self &operator=(const Self &other);

        Self &operator=(Self &&other) noexcept;

        void Swap(Self &other) noexcept;

        inline void swap(Self &other) noexcept
        {
            this->Swap(other);
        }

        inline sharpen::HttpVersion &Version()
        {
            return this->version_;
        }

        inline sharpen::HttpVersion Version() const
        {
            return this->version_;
        }

        inline sharpen::HttpStatusCode &StatusCode()
        {
            return this->status_;
        }

        inline sharpen::HttpStatusCode StatusCode() const
        {
            return this->status_;
        }

        inline sharpen::HttpHeader &Header()
        {
            return this->header_;
        }

        inline const sharpen::HttpHeader &Header() const
        {
            return this->header_;
        }

        inline sharpen::HttpBody &Body()
        {
            return this->body_;
        }

        inline const sharpen::HttpBody &Body() const
        {
            return this->body_;
        }

        ~HttpResponse() noexcept = default;

        void Clear();

        std::size_t ComputeSize() const;

        std::size_t CopyTo(char *buf,std::size_t size) const;

        std::size_t CopyTo(sharpen::ByteBuffer &buf,std::size_t offset) const;

        inline std::size_t CopyTo(sharpen::ByteBuffer &buf) const
        {
            return this->CopyTo(buf,0);
        }

        void ConfigParser(sharpen::HttpParser &parser);
    };
}

#endif