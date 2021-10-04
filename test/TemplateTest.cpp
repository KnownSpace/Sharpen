#include <cstdio>
#include <cassert>

#include <sharpen/TypeTraits.hpp>
#include <sharpen/CompressedPair.hpp>

template<typename _T>
using HasFunc = auto(*)()->decltype(std::declval<_T>().Func());

struct A
{
    void Func();
};

void ValidTest()
{
    std::printf("valid test begin\n");
    int tmp = sharpen::IsMatches<HasFunc,A>::Value;
    std::printf("a has func? %d\n",tmp);
    assert(tmp == true);
    tmp = sharpen::IsMatches<HasFunc,int>::Value;
    std::printf("int has func? %d\n",tmp);
    assert(tmp == false);
    std::printf("valid test pass\n");
}

struct Empty
{};

void IsCompletedTest()
{
    std::printf("is completed type test begin\n");
    class inc;
    int tmp = sharpen::IsCompletedType<inc>::Value;
    std::printf("inc is completed type? %d\n",tmp);
    assert(tmp == false);
    tmp = sharpen::IsCompletedType<int>::Value;
    std::printf("int is completed type? %d\n",tmp);
    assert(tmp == true);
    std::printf("is completed type test pass\n");
}

void IsEmptyTest()
{
    std::printf("is empty type test begin\n");
    int tmp = sharpen::IsEmptyType<Empty>::Value;
    std::printf("empty is empty type? %d\n",tmp);
    assert(tmp == true);
    tmp = sharpen::IsEmptyType<int>::Value;
    std::printf("int is empty type? %d\n",tmp);
    assert(tmp == false);
    std::printf("is empty type test begin\n");
}

void CompressedTest()
{
    std::printf("compressed pair test begin\n");
    sharpen::CompressedPair<Empty,int> p;
    int tmp = sizeof(p) == sizeof(int);
    std::printf("compressed pair size == int size? %d\n",tmp);
    assert(tmp == true);
    std::printf("compressed pair test pass\n");
}

int main(int argc, char const *argv[])
{
    ValidTest();
    IsCompletedTest();
    IsEmptyTest();
    CompressedTest();
    return 0;
}