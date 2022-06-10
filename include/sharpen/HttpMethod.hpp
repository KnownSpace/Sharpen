#pragma once
#ifndef _SHARPEN_HTTPMETHOD_HPP
#define _SHARPEN_HTTPMETHOD_HPP

#include <cstdint>
#include <cstddef>

namespace sharpen
{
    class ByteBuffer;

    enum class HttpMethod : int
    {
        DELETE_,
        GET,
        HEAD,
        POST,
        PUT,
        CONNECT,
        OPTIONS,
        TRACE,
        COPY,
        LOCK,
        MKCOL,
        MOVE,
        PROPFIND,
        PROPPATCH,
        SEARCH,
        UNLOCK,
        BIND,
        REBIND,
        UNBIND,
        ACL,
        REPORT,
        MKACTIVITY,
        CHECKOUT,
        MERGE,
        MSEARCH,
        NOTIFY,
        SUBSCRIBE,
        UNSUBSCRIBE,
        PATCH,
        PURGE,
        MKCALENDAR,
        LINK,
        UNLINK,
        SOURCE
    };

    const char *GetHttpMethodName(sharpen::HttpMethod method);

    void InternalCopyHttpMethodNameToMem(sharpen::HttpMethod method,char *buf,std::size_t offset);

    std::size_t CopyHttpMethodNameTo(sharpen::HttpMethod method,char *buf,std::size_t size);

    std::size_t CopyHttpMethodNameTo(sharpen::HttpMethod method,sharpen::ByteBuffer &buf,std::size_t offset);

    inline std::size_t CopyHttpMethodNameTo(sharpen::HttpMethod method,sharpen::ByteBuffer &buf)
    {
        return sharpen::CopyHttpMethodNameTo(method,buf,0);
    }
}

#endif