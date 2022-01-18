#pragma once
#ifndef _SHARPEN_HTTPREQUESTENCODER_HPP
#define _SHARPEN_HTTPREQUESTENCODER_HPP

#include <stdexcept>

#include "HttpRequest.hpp"
#include "ByteBuffer.hpp"

namespace sharpen
{
    class HttpRequestEncoder
    {
    private:
        
    public:
        inline static sharpen::ByteBuffer Encode(const sharpen::HttpRequest &req)
        {
            sharpen::ByteBuffer buf;
            req.CopyTo(buf);
            return std::move(buf);
        }

        inline static sharpen::Size EncodeTo(const sharpen::HttpRequest &req,sharpen::ByteBuffer &buf,sharpen::Size offset)
        {
            return req.CopyTo(buf,offset);
        }

        inline static sharpen::Size EncodeTo(const sharpen::HttpRequest &req,sharpen::ByteBuffer &buf)
        {
            return req.CopyTo(buf);
        }

        inline static sharpen::Size EncodeTo(const sharpen::HttpRequest &req,char *data,sharpen::Size size)
        {
            return req.CopyTo(data,size);
        }
    };
}

#endif