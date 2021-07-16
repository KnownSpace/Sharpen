#include <sharpen/HttpMethod.hpp>

#include <http_parser.h>

const char *sharpen::GetHttpMethodName(sharpen::HttpMethod method)
{;
    return ::http_method_str(static_cast<http_method>(method));
}