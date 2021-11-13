#pragma once
#ifndef _SHARPEN_HTTPRESPONSEDECODER_HPP
#define _SHARPEN_HTTPRESPONSEDECODER_HPP

#include "HttpParser.hpp"
#include "HttpParseException.hpp"

namespace sharpen
{
    class HttpResponseDecoder
    {
    private:
        using Self = sharpen::HttpResponseDecoder;

        sharpen::HttpParser parser_;
    public:
        HttpResponseDecoder()
            :parser_(sharpen::HttpParser::ParserModel::Response)
        {}

        HttpResponseDecoder(const Self &other)
            :parser_(sharpen::HttpParser::ParserModel::Response)
        {
            (void)other;
        }

        HttpResponseDecoder(Self &&other) noexcept
            :parser_(sharpen::HttpParser::ParserModel::Response)
        {
            (void)other;
        }

        ~HttpResponseDecoder() noexcept = default;

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