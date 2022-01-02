#include <cstdio>
#include <sharpen/BloomFilter.hpp>

int main(int argc, char const *argv[])
{
    std::puts("bloom filter test begin");
    constexpr sharpen::Size count = static_cast<sharpen::Size>(1e4);
    constexpr sharpen::Size bits = 10;
    sharpen::BloomFilter<sharpen::Size> filter{count*bits,bits};
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
    long double p = static_cast<long double>(fake);
    p /= testCount - count;
    p *= 100;
    std::printf("fake/test count=%zu/%zu - %.0Lf%%\n",fake,testCount - count,p);
    std::puts("pass");
    return 0;
}
