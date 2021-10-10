#include <cstdio>
#include <cassert>

#include <sharpen/Option.hpp>

sharpen::Option<int> GetInt(bool i)
{
    if (i)
    {
        return {1};
    }
    return sharpen::NullOpt;
}

struct Test
{

    Test()
    {
        std::printf("%p ctor\n",this);
    }

    Test(const Test &other)
    {
        std::printf("%p copy ctor from %p\n",this,&other);
    }

    Test(Test &&other) noexcept
    {
        std::printf("%p move ctor from %p\n",this,&other);
    }

    Test &operator=(const Test &other)
    {
        std::printf("%p copy = from %p\n",this,&other);
        return *this;
    }

    Test &operator=(Test &&other) noexcept
    {
        std::printf("%p move = from %p\n",this,&other);
        return *this;
    }

    ~Test() noexcept
    {
        std::printf("%p dtor\n",this);
    }
};

sharpen::Option<Test> GetTest(bool i)
{
    if (i)
    {
        return Test{};
    }
    return sharpen::NullOpt;
}

int main()
{
    std::printf("option test begin\n");
    {
        sharpen::Option<int> opt = GetInt(false);
        std::puts("option<int> empty test\n");
        std::printf("opt<int> has value? %d\n",opt.HasValue());
        assert(opt.HasValue() == false);
        try
        {
            int a = opt.Get();
            assert(false);
        }
        catch(const sharpen::BadOptionException &e)
        {}
        opt = GetInt(true);
        std::puts("option<int> value test\n");
        std::printf("opt<int> has value? %d\n",opt.HasValue());
        assert(opt.HasValue() == true);
        try
        {
            int a = opt.Get();
            assert(a == 1);
        }
        catch(const sharpen::BadOptionException &e)
        {
            assert(false);
        }
    }
    {
        sharpen::Option<Test> opt = GetTest(false);
        std::puts("option<Test> empty test\n");
        std::printf("opt has value? %d\n",opt.HasValue());
        assert(opt.HasValue() == false);
        try
        {
            Test &a = opt.Get();
            assert(false);
        }
        catch(const sharpen::BadOptionException &e)
        {}
        std::puts("option<Test> value test\n");
        opt = GetTest(true);
        std::printf("opt has value? %d\n",opt.HasValue());
        assert(opt.HasValue() == true);
        try
        {
            Test &test = opt.Get();
        }
        catch(const sharpen::BadOptionException &e)
        {
            assert(false);
        }
    }
    std::printf("option test pass\n");
    return 0;
}
