#include <sharpen/HttpMethod.hpp>

#include <cstring>
#include <stdexcept>
#include <cassert>

#include <sharpen/ByteBuffer.hpp>

#include <http_parser.h>

const char *sharpen::GetHttpMethodName(sharpen::HttpMethod method)
{;
    return ::http_method_str(static_cast<http_method>(method));
}

void sharpen::InternalCopyHttpMethodNameToMem(sharpen::HttpMethod method,char *buf,sharpen::Size offset)
{
    const char *name = sharpen::GetHttpMethodName(method);
    sharpen::Size size = std::strlen(name);
    std::memcpy(buf + offset,name,size);
}

sharpen::Size sharpen::CopyHttpMethodNameTo(sharpen::HttpMethod method,char *buf,sharpen::Size size)
{
    sharpen::Size len = std::strlen(sharpen::GetHttpMethodName(method));
    if (len > size)
    {
        throw std::length_error("buffer size less than needed");
    }
    sharpen::InternalCopyHttpMethodNameToMem(method,buf,0);
    return len;
}

sharpen::Size sharpen::CopyHttpMethodNameTo(sharpen::HttpMethod method,sharpen::ByteBuffer &buf,sharpen::Size offset)
{
    if (offset > buf.GetSize())
    {
        throw std::length_error("buffer size is wrong");
    }
    sharpen::Size len = std::strlen(sharpen::GetHttpMethodName(method));
    sharpen::Size left = buf.GetSize() - offset;
    if (len > left)
    {
        buf.Extend(len - left);
    }
    sharpen::InternalCopyHttpMethodNameToMem(method,buf.Data(),offset);
    return len;
}