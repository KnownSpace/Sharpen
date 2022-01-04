#include <cstdio>
#include <cassert>
#include <sharpen/IntOps.hpp>

int main(int argc, char const *argv[])
{
    std::puts("int cast test begin");

    int r = sharpen::CheckIntCast<sharpen::Int32>(std::numeric_limits<sharpen::Uint32>::max());
    std::printf("uint32 max cannot cast to int32 ? %d\n",r == 0);
    assert(r == 0);

    r = sharpen::CheckIntCast<sharpen::Int32>(std::numeric_limits<sharpen::Int32>::max());
    std::printf("int32 max can cast to int32 ? %d\n",r == 1);
    assert(r == 1);

    r = sharpen::CheckIntCast<sharpen::Uint64>(std::numeric_limits<sharpen::Size>::max());
    std::printf("size max can cast to uint64 ? %d\n",r == 1);
    assert(r == 1);

    r = sharpen::CheckIntCast<sharpen::Uint32>(std::numeric_limits<sharpen::Size>::max());
    std::printf("size max can cast to uint32 ? %d\n",r);
    assert(r == (sizeof(sharpen::Size) <= sizeof(sharpen::Uint32)));

    std::puts("pass");
    return 0;
}
