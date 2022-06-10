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

        inline static std::size_t EncodeTo(const sharpen::HttpRequest &req,sharpen::ByteBuffer &buf,std::size_t offset)
        {
            return req.CopyTo(buf,offset);
        }

        inline static std::size_t EncodeTo(const sharpen::HttpRequest &req,sharpen::ByteBuffer &buf)
        {
            return req.CopyTo(buf);
        }

        inline static std::size_t EncodeTo(const sharpen::HttpRequest &req,char *data,std::size_t size)
        {
            return req.CopyTo(data,size);
        }
    };
}

#endif