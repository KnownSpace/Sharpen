#pragma once
#ifndef _SHARPEN_HTTPREQUESTDECODER_HPP
#define _SHARPEN_HTTPREQUESTDECODER_HPP

#include "HttpParser.hpp"
#include "HttpParseException.hpp"

namespace sharpen
{
    class HttpRequestDecoder:public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::HttpRequestDecoder;

        sharpen::HttpParser parser_;
    public:
        HttpRequestDecoder()
            :parser_(sharpen::HttpParser::ParserModel::Request)
        {}

        //fake move
        HttpRequestDecoder(Self &&other) noexcept
            :parser_(sharpen::HttpParser::ParserModel::Request)
        {
            (void)other;
        }

        ~HttpRequestDecoder() noexcept = default;

        //fake move
        Self &operator=(Self &&other) noexcept
        {
            (void)other;
            return *this;
        }

        bool IsCompleted() const noexcept
        {
            return this->parser_.IsCompleted();
        }

        sharpen::Size Decode(const char *data,sharpen::Size size)
        {
            this->parser_.Parse(data,size);
            if (this->parser_.IsError())
            {
                throw sharpen::HttpParseException(this->parser_.GetErrorMessage());
            }
            return size;
        }

        void SetCompleted(bool completed)
        {
            this->parser_.SetCompleted(completed);
        }

        void Bind(sharpen::HttpRequest &req)
        {
            req.ConfigParser(this->parser_);
        }
    };
}

#endif