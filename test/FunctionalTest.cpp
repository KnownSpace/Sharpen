#include <cstdio>
#include <sharpen/FunctionalOps.hpp>

struct Simple
{
    void Print()
    {
        std::puts("hello world");
    }

    void PrintInt(int a,int b)
    {
        std::printf("%d %d\n",a,b);
    }

    void PrintConst() const
    {
        std::puts("const");
    }
};

int main()
{
    using ConstSimple = const Simple;
    {
        auto fp = sharpen::TrivialFunction(&Simple::PrintConst);
        fp(nullptr);
    }
    {
        auto fp = sharpen::TrivialFunction(&ConstSimple::PrintConst);
        fp(nullptr);
    }
    {
        auto fp = sharpen::TrivialFunction(&Simple::Print);
        fp(nullptr);
    }
    {
        auto fp = sharpen::TrivialFunction(&Simple::PrintInt);
        fp(nullptr,1,2);
    }
    return 0;
}