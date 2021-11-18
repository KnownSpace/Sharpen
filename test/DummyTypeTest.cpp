#include <cstdio>
#include <cstring>
#include <sharpen/DummyType.hpp>


struct MyTestA
{
    double b;
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
    int a;

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
    dummy.Get<MyTestA>().b = 1;
    odummy = std::move(dummy);
    odummy.Construct<MyTestB>();
    dummy = odummy;
}


int main(int argc, char const *argv[])
{
    DummyTypeTest();
    return 0;
}