#pragma once
#ifndef _SHARPEN_MICRORPCENCODER_HPP
#define _SHARPEN_MICRORPCENCODER_HPP

#include "MicroRpcStack.hpp"
#include "ByteBuffer.hpp"

namespace sharpen
{
    class MicroRpcEncoder
    {
    private:
    public:
        inline static sharpen::ByteBuffer Encode(const sharpen::MicroRpcStack &stack)
        {
            sharpen::ByteBuffer buf;
            stack.CopyTo(buf);
            return buf;
        }

        inline static sharpen::Size EncodeTo(const sharpen::MicroRpcStack &stack,sharpen::ByteBuffer &buf,sharpen::Size offset)
        {
            return stack.CopyTo(buf,offset);
        }

        inline static sharpen::Size EncodeTo(const sharpen::MicroRpcStack &stack,sharpen::ByteBuffer &buf)
        {
            return stack.CopyTo(buf);
        }

        inline static sharpen::Size EncodeTo(const sharpen::MicroRpcStack &stack,char *data,sharpen::Size size)
        {
            return stack.CopyTo(data,size);
        }
    };   
}

#endif