#include <sharpen/HttpVersion.hpp>

#include <cassert>
#include <cstring>
#include <stdexcept>

#include <sharpen/ByteBuffer.hpp>

sharpen::HttpVersion sharpen::GetHttpVersion(std::uint16_t major,std::uint16_t minor)
{
    if (major == 0 && minor == 9)
    {
        return sharpen::HttpVersion::Http0_9;
    }
    if (major == 1 && minor == 0)
    {
        return sharpen::HttpVersion::Http1_0;
    }
    if (major == 1 && minor == 1)
    {
        return sharpen::HttpVersion::Http1_1;
    }
    if (major == 2 && minor ==0)
    {
        return sharpen::HttpVersion::Http2_0;
    }
    return sharpen::HttpVersion::Unkown;
}

const char *sharpen::GetHttpVersionName(sharpen::HttpVersion version)
{
    switch (version)
    {
    case sharpen::HttpVersion::Http0_9:
        return "HTTP/0.9";
    case sharpen::HttpVersion::Http1_0:
        return "HTTP/1.0";
    case sharpen::HttpVersion::Http1_1:
        return "HTTP/1.1";
    case sharpen::HttpVersion::Http2_0:
        return "HTTP/2.0";
    default:
        return "UnKnown";
    }
}

void sharpen::InternalCopyHttpVersionNameToMem(sharpen::HttpVersion version,char *buf,std::size_t offset)
{
    const char *name = sharpen::GetHttpVersionName(version);
    std::size_t size = std::strlen(name);
    std::memcpy(buf + offset,name,size);
}

std::size_t sharpen::CopyHttpVersionNameTo(sharpen::HttpVersion version,char *buf,std::size_t size)
{
    std::size_t len = std::strlen(sharpen::GetHttpVersionName(version));
    if (len > size)
    {
        throw std::length_error("buffer size less than needed");
    }
    sharpen::InternalCopyHttpVersionNameToMem(version,buf,0);
    return len;
}

std::size_t sharpen::CopyHttpVersionNameTo(sharpen::HttpVersion version,sharpen::ByteBuffer &buf,std::size_t offset)
{
    if (offset > buf.GetSize())
    {
        throw std::length_error("buffer size is wrong");
    }
    std::size_t len = std::strlen(sharpen::GetHttpVersionName(version));
    std::size_t left = buf.GetSize() - offset;
    if (left < len)
    {
        buf.Extend(len - left);
    }
    sharpen::InternalCopyHttpVersionNameToMem(version,buf.Data(),offset);
    return len;
}