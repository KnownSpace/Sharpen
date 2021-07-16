#include <sharpen/HttpStatusCode.hpp>

#include <http_parser.h>

const char *sharpen::GetHttpStatusCodeName(sharpen::HttpStatusCode code)
{
    return ::http_status_str(static_cast<http_status>(code));
}