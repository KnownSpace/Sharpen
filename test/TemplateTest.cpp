#include <cstdio>
#include <cassert>
#include <typeinfo>

#include <sharpen/TypeTraits.hpp>
#include <sharpen/CompressedPair.hpp>
#include <sharpen/DummyType.hpp>

template<typename _T>
using HasFunc = auto(*)()->decltype(std::declval<_T>().Func());

template<typename _T1,typename _T2>
using HasAdd = auto(*)() -> decltype(std::declval<_T1>() + std::declval<_T2>(),std::declval<_T1>() - std::declval<_T2>());

struct A
{
    void Func();
};

void ValidTest()
{
    std::printf("valid test begin\n");
    bool tmp = sharpen::IsMatches<HasFunc,A>::Value;
    std::printf("a has func? %d\n",tmp);
    assert(tmp == true);
    tmp = sharpen::IsMatches<HasFunc,int>::Value;
    std::printf("int has func? %d\n",tmp);
    assert(tmp == false);
    std::printf("valid test pass\n");
    sharpen::IsMatches<HasAdd,int,double>::Value;
}

struct Empty
{};

void IsCompletedTest()
{
    std::printf("is completed type test begin\n");
    class inc;
    bool tmp = sharpen::IsCompletedType<inc>::Value;
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
    bool tmp = sharpen::IsEmptyType<Empty>::Value;
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
    bool tmp = sizeof(p) == sizeof(int);
    std::printf("compressed pair size == int size? %d\n",tmp);
    assert(tmp == true);
    std::printf("compressed pair test pass\n");
}

void TypeListTest()
{
    using TL = sharpen::TypeList<int,char,double>;
    std::printf("TL has %zu types\n",TL::Size);
    std::printf("first is %s\n",typeid(TL::At<0>).name());
    std::printf("second is %s\n",typeid(TL::At<1>).name());
    std::printf("thrid is %s\n",typeid(TL::At<2>).name());
    std::printf("now we push a float to TL\n");
    using NewTL1 = TL::PushBack<float>;
    std::printf("TL has %zu types\n",NewTL1::Size);
    std::printf("first is %s\n",typeid(NewTL1::At<0>).name());
    std::printf("second is %s\n",typeid(NewTL1::At<1>).name());
    std::printf("thrid is %s\n",typeid(NewTL1::At<2>).name());
    std::printf("fourth is %s\n",typeid(NewTL1::At<3>).name());
    std::printf("now we remove int from TL\n");
    using NewTL2 = NewTL1::Remove<int>;
    std::printf("TL has %zu types\n",NewTL2::Size);
    std::printf("first is %s\n",typeid(NewTL2::At<0>).name());
    std::printf("second is %s\n",typeid(NewTL2::At<1>).name());
    std::printf("thrid is %s\n",typeid(NewTL2::At<2>).name());
    std::printf("TL contain int? %d\n",NewTL2::Contain<int>::Value);
}

struct MyTestA
{
    MyTestA()
    {
        std::printf("A CTOR\n");
    }

    ~MyTestA() noexcept
    {
        std::printf("A DTOR\n");
    }

    MyTestA(const MyTestA &)
    {
        std::printf("A COPY CTOR\n");
    }

    MyTestA(MyTestA &&) noexcept
    {
        std::printf("A MOVE CTOR\n");
    }

    MyTestA &operator=(const MyTestA &)
    {
        std::printf("A COPY ASSIGN\n");
        return *this;
    }

    MyTestA &operator=(MyTestA &&) noexcept
    {
        std::printf("A MOVE ASSIGN\n");
        return *this;
    }
};

struct MyTestB
{
    MyTestB()
    {
        std::printf("B CTOR\n");
    }

    ~MyTestB() noexcept
    {
        std::printf("B DTOR\n");
    }
    MyTestB(const MyTestB &)
    {
        std::printf("B COPY CTOR\n");
    }

    MyTestB(MyTestB &&) noexcept
    {
        std::printf("B MOVE CTOR\n");
    }

    MyTestB &operator=(const MyTestB &)
    {
        std::printf("B COPY ASSIGN\n");
        return *this;
    }

    MyTestB &operator=(MyTestB &&) noexcept
    {
        std::printf("B MOVE ASSIGN\n");
        return *this;
    }
};

void DummyTypeTest()
{
    sharpen::DummyType<MyTestA,MyTestB> dummy,odummy;
    dummy.Construct<MyTestA>();
    odummy = std::move(dummy);
    odummy.Construct<MyTestB>();
    dummy = odummy;
}

int main(int argc, char const *argv[])
{
    ValidTest();
    IsCompletedTest();
    IsEmptyTest();
    CompressedTest();
    TypeListTest();
    DummyTypeTest();
    return 0;
}