#include <cstdio>
#include <sharpen/BloomFilter.hpp>

int main(int argc, char const *argv[])
{
    std::puts("bloom filter test begin");
    constexpr sharpen::Size count = static_cast<sharpen::Size>(1e4);
    sharpen::BloomFilter<sharpen::Size> filter{count*10,10};
    for (sharpen::Size i = 0; i < count; ++i)
    {
        filter.Add(i);
    }
    for (sharpen::Size i = 0; i < count; i++)
    {
        assert(filter.Containe(i));
    }
    sharpen::Size fake{0};
    constexpr sharpen::Size testCount = static_cast<sharpen::Size>(1e9);
    for (sharpen::Size i = count; i < testCount; ++i)
    {
        if(filter.Containe(i))
        {
            fake += 1;
        }
    }
    std::printf("fake/test count=%zu/%zu\n",fake,testCount - count);
    std::puts("pass");
    return 0;
}
