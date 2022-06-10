#include <sharpen/HttpStatusCode.hpp>

#include <cstring>
#include <stdexcept>

#include <sharpen/ByteBuffer.hpp>

#include <llhttp.h>


const char *sharpen::GetHttpStatusCodeName(sharpen::HttpStatusCode code)
{
    switch (code) 
    {
        case sharpen::HttpStatusCode::CONTINUE: return "Continue";
        case sharpen::HttpStatusCode::SWITCHING_PROTOCOLS: return "Switching Protocols";
        case sharpen::HttpStatusCode::PROCESSING: return "Processing";
        case sharpen::HttpStatusCode::OK: return "OK";
        case sharpen::HttpStatusCode::CREATED: return "Created";
        case sharpen::HttpStatusCode::ACCEPTED: return "Accepted";
        case sharpen::HttpStatusCode::NON_AUTHORITATIVE_INFORMATION: return"NON-Authoritative Information";
        case sharpen::HttpStatusCode::NO_CONTENT: return "No Content";
        case sharpen::HttpStatusCode::RESET_CONTENT: return "Reset content";
        case sharpen::HttpStatusCode::PARTIAL_CONTENT: return "Partial Content";
        case sharpen::HttpStatusCode::MULTI_STATUS: return "Multi-Status";
        case sharpen::HttpStatusCode::ALREADY_REPORTED: return "Already Reported";
        case sharpen::HttpStatusCode::IM_USED: return "IM Used";
        case sharpen::HttpStatusCode::MULTIPLE_CHOICES: return "Multiple Choices";
        case sharpen::HttpStatusCode::MOVED_PERMANENTLY: return "Moved Permanently";
        case sharpen::HttpStatusCode::FOUND: return "Found";
        case sharpen::HttpStatusCode::SEE_OTHER: return "See Other";
        case sharpen::HttpStatusCode::NOT_MODIFIED: return "Not Modified";
        case sharpen::HttpStatusCode::USE_PROXY: return "Use Proxy";
        case sharpen::HttpStatusCode::TEMPORARY_REDIRECT: return "Temporary Redirect";
        case sharpen::HttpStatusCode::PERMANENT_REDIRECT: return "Permanent Redirect";
        case sharpen::HttpStatusCode::FORBIDDEN: return "Forbidden";
        case sharpen::HttpStatusCode::NOT_FOUND: return "Not Found";
        case sharpen::HttpStatusCode::METHOD_NOT_ALLOWED: return "Method Not Allowed";
        case sharpen::HttpStatusCode::NOT_ACCEPTABLE: return "Not Acceptable";
        case sharpen::HttpStatusCode::PROXY_AUTHENTICATION_REQUIRED: return "Proxy Authentication Required";
        case sharpen::HttpStatusCode::REQUEST_TIMEOUT: return "Request Timeout";
        case sharpen::HttpStatusCode::CONFLICT: return "Conflict";
        case sharpen::HttpStatusCode::GONE: return "Gone";
        case sharpen::HttpStatusCode::LENGTH_REQUIRED: return "Length Required";
        case sharpen::HttpStatusCode::PRECONDITION_FAILED: return "Precondition Failed";
        case sharpen::HttpStatusCode::PAYLOAD_TOO_LARGE: return "Payload Too Large";
        case sharpen::HttpStatusCode::URI_TOO_LONG: return "URI Too Long";
        case sharpen::HttpStatusCode::UNSUPPORTED_MEDIA_TYPE: return "Unsupproted Media Type";
        case sharpen::HttpStatusCode::RANGE_NOT_SATISFIABLE: return "Range Not Stisfiable";
        case sharpen::HttpStatusCode::EXPECTATION_FAILED: return "Expectation Failed";
        case sharpen::HttpStatusCode::MISDIRECTED_REQUEST: return "Misdirected Request";
        case sharpen::HttpStatusCode::UNPROCESSABLE_ENTITY: return "Unprocessable Entity";
        case sharpen::HttpStatusCode::LOCKED: return "Locked";
        case sharpen::HttpStatusCode::FAILED_DEPENDENCY: return "Failed Denpendency";
        case sharpen::HttpStatusCode::UPGRADE_REQUIRED: return "Upgrade Required";
        case sharpen::HttpStatusCode::PRECONDITION_REQUIRED: return "Precondition Required";
        case sharpen::HttpStatusCode::TOO_MANY_REQUESTS: return "Too Many Requests";
        case sharpen::HttpStatusCode::REQUEST_HEADER_FIELDS_TOO_LARGE: return "Request Header Fields Too Large";
        case sharpen::HttpStatusCode::UNAVAILABLE_FOR_LEGAL_REASONS: return "Unavaliable For Legal Reasons";
        case sharpen::HttpStatusCode::INTERNAL_SERVER_ERROR: return "Internal Server Error";
        case sharpen::HttpStatusCode::NOT_IMPLEMENTED: return "Not Implemented";
        case sharpen::HttpStatusCode::BAD_GATEWAY: return "Bad Gateway";
        case sharpen::HttpStatusCode::SERVICE_UNAVAILABLE: return "Service Unavailable";
        case sharpen::HttpStatusCode::GATEWAY_TIMEOUT: return "Gateway Timeout";
        case sharpen::HttpStatusCode::HTTP_VERSION_NOT_SUPPORTED: return "HTTP Version Not Supported";
        case sharpen::HttpStatusCode::VARIANT_ALSO_NEGOTIATES: return "Variant Also Negotiates";
        case sharpen::HttpStatusCode::INSUFFICIENT_STORAGE: return "Insufficient Storage";
        case sharpen::HttpStatusCode::LOOP_DETECTED: return "Loop Detected";
        case sharpen::HttpStatusCode::NOT_EXTENDED: return "Not Extended";
        case sharpen::HttpStatusCode::NETWORK_AUTHENTICATION_REQUIRED: return "Network Authentication Required";
        default: return "Unknown";
    }
}

void sharpen::InternalCopyHttpStatusCodeNameToMem(sharpen::HttpStatusCode code,char *buf,std::size_t offset)
{
    const char *name = sharpen::GetHttpStatusCodeName(code);
    std::size_t len = std::strlen(name);
    std::memcpy(buf + offset,name,len);
}

std::size_t sharpen::CopyHttpStatusCodeNameTo(sharpen::HttpStatusCode code,char *buf,std::size_t size)
{
    std::size_t len = std::strlen(sharpen::GetHttpStatusCodeName(code));
    if (len > size)
    {
        throw std::length_error("buffer size less than needed");
    }
    sharpen::InternalCopyHttpStatusCodeNameToMem(code,buf,0);
    return len;
}

std::size_t sharpen::CopyHttpStatusCodeNameTo(sharpen::HttpStatusCode code,sharpen::ByteBuffer &buf,std::size_t offset)
{
    if (offset > buf.GetSize())
    {
        throw std::length_error("buffer size is wrong");
    }
    std::size_t len = std::strlen(sharpen::GetHttpStatusCodeName(code));
    std::size_t left = buf.GetSize() - offset;
    if (len > left)
    {
        buf.Extend(len - left);
    }
    sharpen::InternalCopyHttpStatusCodeNameToMem(code,buf.Data(),offset);
    return len;
}