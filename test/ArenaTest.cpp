#include <sharpen/Arena.hpp>
#include <cassert>
#include <cstdio>
#include <sharpen/StopWatcher.hpp>

class MyTestClass
{
private:
    int *p_;

public:
    MyTestClass(int *p)
        : p_(p)
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
        auto array = arena.MakeUniqueArray<MyTestClass>(10, &p);
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
        auto obj = arena.MakeSharedArray<MyTestClass>(10, &p);
        auto copy = obj;
    }
    std::puts("pass");
    std::puts("benchmark");
    constexpr size_t count = static_cast<size_t>(1e8);
    sharpen::StopWatcher sw;
    sw.Begin();
    {
        for (size_t i = 0; i < count; i++)
        {
            std::unique_ptr<size_t> p(new size_t(i));
        }
    }
    sw.Stop();
    std::printf("malloc using %zu tu\n",static_cast<size_t>(sw.Compute()));
    sw.Begin();
    {
        sharpen::Arena arena;
        for (size_t i = 0; i < count; i++)
        {
            auto p = arena.MakeUniqueObject<size_t>(i);
        }
    }
    sw.Stop();
    std::printf("arena using %zu tu\n",static_cast<size_t>(sw.Compute()));
    std::printf("1 sec = %zu tu\n",static_cast<size_t>(CLOCKS_PER_SEC));
    return 0;
}
