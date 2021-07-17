#pragma once
#ifndef _SHARPEN_HTTPMETHOD_HPP
#define _SHARPEN_HTTPMETHOD_HPP

#include "TypeDef.hpp"

namespace sharpen
{
    class ByteBuffer;

    enum class HttpMethod:int
    {
        DELETE,
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

    void InternalCopyHttpMethodNameToMem(sharpen::HttpMethod method,char *buf,sharpen::Size offset);

    void CopyHttpMethodNameTo(sharpen::HttpMethod method,char *buf,sharpen::Size size);

    void CopyHttpMethodNameTo(sharpen::HttpMethod method,sharpen::ByteBuffer &buf,sharpen::Size offset);

    inline void CopyHttpMethodNameTo(sharpen::HttpMethod method,sharpen::ByteBuffer &buf)
    {
        sharpen::CopyHttpMethodNameTo(method,buf,0);
    }
}

#endif