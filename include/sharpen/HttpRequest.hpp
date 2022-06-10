#pragma once
#ifndef _SHARPEN_HTTPREQUEST_HPP
#define _SHARPEN_HTTP_REQUEST_HPP

#include <string>

#include "HttpMethod.hpp"
#include "HttpHeader.hpp"
#include "HttpBody.hpp"
#include "HttpVersion.hpp"

namespace sharpen
{
    class ByteBuffer;

    class HttpParser;

    class HttpRequestDecoder;

    class HttpRequest
    {
    private:
        using Self = sharpen::HttpRequest;

        //METHOD URL VERSION\r\n
        //field: value\r\n
        //...
        //field: value\r\n\r\n
        //body
        sharpen::HttpMethod method_;
        
        std::string url_;

        sharpen::HttpVersion version_;

        sharpen::HttpHeader header_;

        sharpen::HttpBody body_;

    public:
        HttpRequest();

        explicit HttpRequest(sharpen::HttpMethod method,std::string url,sharpen::HttpVersion version);

        HttpRequest(const Self &other);

        HttpRequest(Self &&other) noexcept;

        Self &operator=(const Self &other);

        Self &operator=(Self &&other) noexcept;

        void Swap(Self &other) noexcept;

        inline void swap(Self &other) noexcept
        {
            this->Swap(other);
        }

        inline sharpen::HttpMethod &Method()
        {
            return this->method_;
        }

        inline sharpen::HttpMethod Method() const
        {
            return this->method_;
        }

        inline std::string &Url()
        {
            return this->url_;
        }

        inline const std::string &Url() const
        {
            return this->url_;
        }

        inline sharpen::HttpVersion &Version()
        {
            return this->version_;
        }

        inline sharpen::HttpVersion Version() const
        {
            return this->version_;
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

        ~HttpRequest() noexcept = default;

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