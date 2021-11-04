#include <sharpen/Arena.hpp>
#include <cassert>
#include <cstdio>
#include <sharpen/StopWatcher.hpp>
#include <vector>

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
    std::puts("benchmark begin");
    constexpr size_t count = static_cast<size_t>(1e8);
    sharpen::StopWatcher sw;
    {

        sw.Begin();
        {
            std::vector<std::unique_ptr<int>> ptrs{count};
            for (int i = 0; i < count; i++)
            {
                ptrs[i] = std::unique_ptr<int>(new int(i));
            }
        }
        sw.Stop();
    }
    size_t mallocUse = static_cast<size_t>(sw.Compute());
    {
        sw.Begin();
        {
            sharpen::Arena arena;
            std::vector<std::unique_ptr<int,sharpen::Arena::ObjectDeletor<int>>> ptrs{count};
            for (int i = 0; i < count; i++)
            {
                ptrs[i] = arena.MakeUniqueObject<int>(i);
            }
        }
        sw.Stop();
    }
    std::printf("alloc using %zu tu\n", mallocUse);
    std::printf("arena using %zu tu\n", static_cast<size_t>(sw.Compute()));
    std::printf("1 sec = %zu tu\n", static_cast<size_t>(CLOCKS_PER_SEC));
    return 0;
}
