#include <cstdio>
#include <cassert>
#include <sharpen/BufferOps.hpp>

int main(int argc, char const *argv[])
{
    std::puts("crc16-modbus test begin");
    char buf[] = "123456789";
    std::uint32_t checksum = sharpen::Crc16(buf,sizeof(buf) - 1);
    std::printf("check sum is %X\n",checksum);
    assert(checksum == 0x4B37);
    std::puts("pass");
    checksum = sharpen::Adler32(buf,sizeof(buf) - 1);
    std::printf("check sum is %X\n",checksum);
    assert(checksum == 0x91E01DE);
    std::puts("pass");
    return 0;
}
