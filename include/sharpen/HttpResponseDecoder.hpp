#pragma once
#ifndef _SHARPEN_HTTPRESPONSEDECODER_HPP
#define _SHARPEN_HTTPRESPONSEDECODER_HPP

#include "HttpParser.hpp"
#include "HttpParseException.hpp"
#include "ByteBuffer.hpp"

namespace sharpen
{
    class HttpResponseDecoder:public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::HttpResponseDecoder;

        sharpen::HttpParser parser_;
    public:
        HttpResponseDecoder()
            :parser_(sharpen::HttpParser::ParserModel::Response)
        {}

        //fake move
        HttpResponseDecoder(Self &&other) noexcept
            :parser_(sharpen::HttpParser::ParserModel::Response)
        {
            (void)other;
        }

        ~HttpResponseDecoder() noexcept = default;

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

        inline sharpen::Size Decode(const sharpen::ByteBuffer &buf,sharpen::Size offset)
        {
            return this->Decode(buf.Data() + offset,buf.GetSize() - offset);
        }

        inline sharpen::Size Decode(const sharpen::ByteBuffer &buf)
        {
            return this->Decode(buf,0);
        }

        void SetCompleted(bool completed)
        {
            this->parser_.SetCompleted(completed);
        }

        void Bind(sharpen::HttpResponse &res)
        {
            res.ConfigParser(this->parser_);
        }
    };
}

#endif