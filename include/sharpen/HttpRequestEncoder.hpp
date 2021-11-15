#pragma once
#ifndef _SHARPEN_HTTPREQUESTENCODER_HPP
#define _SHARPEN_HTTPREQUESTENCODER_HPP

#include "HttpRequest.hpp"
#include "ByteBuffer.hpp"

namespace sharpen
{
    class HttpRequestEncoder
    {
    private:
        
    public:
        static sharpen::ByteBuffer Encode(const sharpen::HttpRequest &req)
        {
            sharpen::ByteBuffer buf;
            req.CopyTo(buf);
            return std::move(buf);
        }

        static sharpen::Size EncodeTo(const sharpen::HttpRequest &req,sharpen::ByteBuffer &buf)
        {
            return req.CopyTo(buf);
        }
    };
}

#endif