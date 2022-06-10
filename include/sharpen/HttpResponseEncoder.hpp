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
        inline static sharpen::ByteBuffer Encode(const sharpen::HttpResponse &req)
        {
            sharpen::ByteBuffer buf;
            req.CopyTo(buf);
            return std::move(buf);
        }

        inline static std::size_t EncodeTo(const sharpen::HttpResponse &res,sharpen::ByteBuffer &buf,std::size_t offset)
        {
            return res.CopyTo(buf,offset);
        }

        inline static std::size_t EncodeTo(const sharpen::HttpResponse &res,sharpen::ByteBuffer &buf)
        {
            return res.CopyTo(buf);
        }

        inline static std::size_t EncodeTo(const sharpen::HttpResponse &res,char *data,std::size_t size)
        {
            return res.CopyTo(data,size);
        }
    };
}

#endif