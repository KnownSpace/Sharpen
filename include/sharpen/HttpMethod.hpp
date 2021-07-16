#pragma once
#ifndef _SHARPEN_HTTPMETHOD_HPP
#define _SHARPEN_HTTPMETHOD_HPP

namespace sharpen
{
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
}

#endif