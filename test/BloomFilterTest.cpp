#include <cstdio>
#include <sharpen/BloomFilter.hpp>

int main(int argc, char const *argv[])
{
    std::puts("bloom filter test begin");
    constexpr sharpen::Size count = static_cast<sharpen::Size>(1e5);
    sharpen::BloomFilter<int> filter{count,8};
    for (int i = 0; i < count; ++i)
    {
        filter.Add(i);
    }
    for (int i = 0; i < count; i++)
    {
        assert(filter.Containe(i));
    }
    constexpr sharpen::Size notExist = count;
    std::printf("containe %zu ? %d\n",notExist,filter.Containe(notExist));
    std::puts("pass");
    return 0;
}
