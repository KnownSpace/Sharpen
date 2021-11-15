#pragma once
#ifndef _SHARPEN_HTTPRESPONSEENCODER_HPP
#define _SHARPEN_HTTPRESPONSEENCODER_HPP

#include "HttpResponse.hpp"
#include "ByteBuffer.hpp"

namespace sharpen
{
    class HttpResponseEncoder
    {
    private:
        
    public:
        static sharpen::ByteBuffer Encode(const sharpen::HttpResponse &req)
        {
            sharpen::ByteBuffer buf;
            req.CopyTo(buf);
            return std::move(buf);
        }

        static sharpen::Size EncodeTo(const sharpen::HttpResponse &res,sharpen::ByteBuffer &buf)
        {
            return res.CopyTo(buf);
        }
    };
}

#endif