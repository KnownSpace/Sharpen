#include <sharpen/Arena.hpp>
#include <cassert>
#include <cstdio>

class MyTestClass
{
private:
    int *p_;
public:
    MyTestClass(int *p)
        :p_(p)
    {
        *p += 1;
    }

    ~MyTestClass() noexcept
    {
        *this->p_ -= 1;
    }
};

int main(int argc, char const *argv[])
{
    std::puts("begin arena test");
    int p = 0;
    std::puts("testing unique object");
    {
        sharpen::Arena arena;
        auto obj = arena.MakeUniqueObject<MyTestClass>(&p);
    }
    assert(p == 0);
    std::puts("testing unique array");
    {
        sharpen::Arena arena;
        auto array = arena.MakeUniqueArray<MyTestClass>(10,&p);
    }
    assert(p == 0);
    std::puts("testing shared object");
    {
        sharpen::Arena arena;
        auto obj = arena.MakeSharedObject<MyTestClass>(&p);
        auto copy = obj;
    }
    assert(p == 0);
    std::puts("testing shared array");
    {
        sharpen::Arena arena;
        auto obj = arena.MakeSharedArray<MyTestClass>(10,&p);
        auto copy = obj;
    }
    std::puts("pass");
    return 0;
}
