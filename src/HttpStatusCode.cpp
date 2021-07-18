#include <sharpen/HttpStatusCode.hpp>

#include <cstring>
#include <stdexcept>

#include <sharpen/ByteBuffer.hpp>

#include <http_parser.h>

const char *sharpen::GetHttpStatusCodeName(sharpen::HttpStatusCode code)
{
    return ::http_status_str(static_cast<http_status>(code));
}

void sharpen::InternalCopyHttpStatusCodeNameToMem(sharpen::HttpStatusCode code,char *buf,sharpen::Size offset)
{
    const char *name = sharpen::GetHttpStatusCodeName(code);
    sharpen::Size len = std::strlen(name);
    std::memcpy(buf + offset,name,len);
}

sharpen::Size sharpen::CopyHttpStatusCodeNameTo(sharpen::HttpStatusCode code,char *buf,sharpen::Size size)
{
    sharpen::Size len = std::strlen(sharpen::GetHttpStatusCodeName(code));
    if (len > size)
    {
        throw std::length_error("buffer size less than needed");
    }
    sharpen::InternalCopyHttpStatusCodeNameToMem(code,buf,0);
    return len;
}

sharpen::Size sharpen::CopyHttpStatusCodeNameTo(sharpen::HttpStatusCode code,sharpen::ByteBuffer &buf,sharpen::Size offset)
{
    if (offset > buf.GetSize())
    {
        throw std::length_error("buffer size is wrong");
    }
    sharpen::Size len = std::strlen(sharpen::GetHttpStatusCodeName(code));
    sharpen::Size left = buf.GetSize() - offset;
    if (len > left)
    {
        buf.Extend(len - left);
    }
    sharpen::InternalCopyHttpStatusCodeNameToMem(code,buf.Data(),offset);
    return len;
}