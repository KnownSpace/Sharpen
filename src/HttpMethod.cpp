#include <sharpen/HttpMethod.hpp>

#include <cstring>
#include <stdexcept>
#include <cassert>

#include <sharpen/ByteBuffer.hpp>

#include <llhttp.h>

const char *sharpen::GetHttpMethodName(sharpen::HttpMethod method)
{
    return ::llhttp_method_name(static_cast<llhttp_method>(method));
}

void sharpen::InternalCopyHttpMethodNameToMem(sharpen::HttpMethod method,char *buf,std::size_t offset)
{
    const char *name = sharpen::GetHttpMethodName(method);
    std::size_t size = std::strlen(name);
    std::memcpy(buf + offset,name,size);
}

std::size_t sharpen::CopyHttpMethodNameTo(sharpen::HttpMethod method,char *buf,std::size_t size)
{
    std::size_t len = std::strlen(sharpen::GetHttpMethodName(method));
    if (len > size)
    {
        throw std::length_error("buffer size less than needed");
    }
    sharpen::InternalCopyHttpMethodNameToMem(method,buf,0);
    return len;
}

std::size_t sharpen::CopyHttpMethodNameTo(sharpen::HttpMethod method,sharpen::ByteBuffer &buf,std::size_t offset)
{
    if (offset > buf.GetSize())
    {
        throw std::length_error("buffer size is wrong");
    }
    std::size_t len = std::strlen(sharpen::GetHttpMethodName(method));
    std::size_t left = buf.GetSize() - offset;
    if (len > left)
    {
        buf.Extend(len - left);
    }
    sharpen::InternalCopyHttpMethodNameToMem(method,buf.Data(),offset);
    return len;
}