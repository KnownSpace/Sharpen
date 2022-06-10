#pragma once
#ifndef _SHARPEN_HTTPVERSION_HPP

#include <cstdint>
#include <cstddef>

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

    sharpen::HttpVersion GetHttpVersion(std::uint16_t major,std::uint16_t minor);

    const char *GetHttpVersionName(sharpen::HttpVersion version);

    void InternalCopyHttpVersionNameToMem(sharpen::HttpVersion version,char *buf,std::size_t offset);

    std::size_t CopyHttpVersionNameTo(sharpen::HttpVersion version,char *buf,std::size_t size);

    std::size_t CopyHttpVersionNameTo(sharpen::HttpVersion version,sharpen::ByteBuffer &buf,std::size_t offset);

    inline std::size_t CopyHttpVersionNameTo(sharpen::HttpVersion version,sharpen::ByteBuffer &buf)
    {
        return sharpen::CopyHttpVersionNameTo(version,buf,0);
    }
}

#endif