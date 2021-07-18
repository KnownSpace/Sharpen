#pragma once
#ifndef _SHARPEN_HTTPVERSION_HPP

#include "TypeDef.hpp"

namespace sharpen
{
    class ByteBuffer;

    enum class HttpVersion
    {
        Http0_9,
        Http1_0,
        Http1_1,
        Http2_0,
        Unkown,
    };

    sharpen::HttpVersion GetHttpVersion(sharpen::Uint16 major,sharpen::Uint16 minor);

    const char *GetHttpVersionName(sharpen::HttpVersion version);

    void InternalCopyHttpVersionNameToMem(sharpen::HttpVersion version,char *buf,sharpen::Size offset);

    sharpen::Size CopyHttpVersionNameTo(sharpen::HttpVersion version,char *buf,sharpen::Size size);

    sharpen::Size CopyHttpVersionNameTo(sharpen::HttpVersion version,sharpen::ByteBuffer &buf,sharpen::Size offset);

    inline sharpen::Size CopyHttpVersionNameTo(sharpen::HttpVersion version,sharpen::ByteBuffer &buf)
    {
        return sharpen::CopyHttpVersionNameTo(version,buf,0);
    }
}

#endif