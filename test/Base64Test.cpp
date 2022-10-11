#include <cstdio>
#include <cstring>
#include <cassert>
#include <sharpen/BufferOps.hpp>

int main(int argc,char const *argv[])
{
    std::puts("base64 test begin");
    {
        char str[] = "ABC";
        char buf[5] = {0};
        sharpen::Base64Encode(buf,sizeof(buf) - 1,str,sizeof(str) - 1);
        assert(!std::strcmp("QUJD",buf));
    }
    {
        char buf[] = "QUJD";
        char str[4] = {0};
        sharpen::Base64Decode(str,sizeof(str) - 1,buf,sizeof(buf) - 1);
        assert(!std::strcmp("ABC",str));
    }
    std::puts("pass");
    return 0;
}
