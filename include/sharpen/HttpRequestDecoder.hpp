#pragma once
#ifndef _SHARPEN_HTTPREQUESTDECODER_HPP
#define _SHARPEN_HTTPREQUESTDECODER_HPP

#include "HttpParser.hpp"
#include "HttpParseException.hpp"

namespace sharpen
{
    class HttpRequestDecoder
    {
    private:
        using Self = sharpen::HttpRequestDecoder;

        sharpen::HttpParser parser_;
    public:
        HttpRequestDecoder()
            :parser_(sharpen::HttpParser::ParserModel::Request)
        {}

        HttpRequestDecoder(const Self &other)
            :parser_(sharpen::HttpParser::ParserModel::Request)
        {
            (void)other;
        }

        HttpRequestDecoder(Self &&other) noexcept
            :parser_(sharpen::HttpParser::ParserModel::Request)
        {
            (void)other;
        }

        ~HttpRequestDecoder() noexcept = default;

        Self &operator=(const Self &other)
        {
            (void)other;
            return *this;
        }

        Self &operator=(Self &&other) noexcept
        {
            (void)other;
            return *this;
        }

        bool IsCompleted() const noexcept
        {
            return this->parser_.IsCompleted();
        }

        void Decode(const char *data,sharpen::Size size)
        {
            this->parser_.Parse(data,size);
            if (this->parser_.IsError())
            {
                throw sharpen::HttpParseException(this->parser_.GetErrorMessage());
            }
        }

        sharpen::HttpParser &GetParser() noexcept
        {
            return this->parser_;
        }

        const sharpen::HttpParser &GetParser() const noexcept
        {
            return this->parser_;
        }
    };
}

#endif