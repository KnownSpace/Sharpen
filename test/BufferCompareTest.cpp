#include <cstdio>
#include <cassert>
#include <sharpen/BufferOps.hpp>

int main(int argc, char const *argv[])
{
    std::puts("buffer compare test begin");
    char buf1[] = "1234";
    char buf2[] = "12345";
    sharpen::Int32 r = sharpen::BufferCompare(buf1, sizeof(buf1), buf2, sizeof(buf2));
    std::printf("is buf1 < buf2 ? %d\n", r == -1);
    assert(r == -1);

    r = sharpen::BufferCompare(buf2, sizeof(buf2), buf1, sizeof(buf1));
    std::printf("is buf2 > buf1 ? %d\n", r == 1);
    assert(r == 1);

    char buf3[] = "1235";
    r = sharpen::BufferCompare(buf2, sizeof(buf2), buf3, sizeof(buf3));
    std::printf("is buf2 < buf3 ? %d\n", r == -1);
    assert(r == -1);

    r = sharpen::BufferCompare(buf3, sizeof(buf3), buf2, sizeof(buf2));
    std::printf("is buf3 > buf2 ? %d\n", r == 1);
    assert(r == 1);

    char buf4[] = "1235";
    r = sharpen::BufferCompare(buf3, sizeof(buf3), buf4, sizeof(buf4));
    std::printf("is buf3 == buf4 ? %d\n", r == 0);
    assert(r == 0);

    r = sharpen::BufferCompare(buf4, sizeof(buf4), buf3, sizeof(buf3));
    std::printf("is buf4 == buf3 ? %d\n", r == 0);
    assert(r == 0);

    char buf5[] = "2";
    r = sharpen::BufferCompare(buf5,sizeof(buf5),buf1,sizeof(buf1));
    std::printf("is buf5 > buf1 ? %d\n", r == 1);
    assert(r == 1);
    
    std::puts("pass");
    return 0;
}
