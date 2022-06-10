#include <cstdio>
#include <cassert>
#include <sharpen/IntOps.hpp>

int main(int argc, char const *argv[])
{
    std::puts("int cast test begin");

    int r = sharpen::CheckIntCast<std::int32_t>(std::numeric_limits<std::uint32_t>::max());
    std::printf("uint32 max cannot cast to int32 ? %d\n",r == 0);
    assert(r == 0);

    r = sharpen::CheckIntCast<std::int32_t>(std::numeric_limits<std::int32_t>::max());
    std::printf("int32 max can cast to int32 ? %d\n",r == 1);
    assert(r == 1);

    r = sharpen::CheckIntCast<std::uint64_t>(std::numeric_limits<std::size_t>::max());
    std::printf("size max can cast to uint64 ? %d\n",r == 1);
    assert(r == 1);

    r = sharpen::CheckIntCast<std::uint32_t>(std::numeric_limits<std::size_t>::max());
    std::printf("size max can cast to uint32 ? %d\n",r);
    assert(r == (sizeof(std::size_t) <= sizeof(std::uint32_t)));

    r = sharpen::CheckIntCast<std::uint32_t>(std::numeric_limits<std::int32_t>::min());
    std::printf("int32 min can cast to uint32 ? %d\n",r);
    assert(r == 0);

    r = sharpen::CheckIntCast<std::uint32_t>(std::numeric_limits<std::int64_t>::min());
    std::printf("int64 min can cast to uint32 ? %d\n",r);
    assert(r == 0);

    std::puts("pass");
    return 0;
}
