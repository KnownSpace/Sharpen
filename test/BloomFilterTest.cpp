#include <cstdio>
#include <sharpen/BloomFilter.hpp>

int main(int argc, char const *argv[])
{
    std::puts("bloom filter test begin");
    constexpr std::size_t count = static_cast<std::size_t>(1e4);
    constexpr std::size_t bits = 10;
    sharpen::BloomFilter<std::size_t> filter{count*bits,bits};
    for (std::size_t i = 0; i < count; ++i)
    {
        filter.Add(i);
    }
    for (std::size_t i = 0; i < count; i++)
    {
        assert(filter.Containe(i));
    }
    std::size_t fake{0};
    constexpr std::size_t testCount = static_cast<std::size_t>(1e5);
    for (std::size_t i = count; i < testCount; ++i)
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
