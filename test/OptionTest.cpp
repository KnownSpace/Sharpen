#include <cstdio>
#include <cassert>

#include <sharpen/Optional.hpp>

sharpen::Optional<int> GetInt(bool i)
{
    if (i)
    {
        return {1};
    }
    return sharpen::EmptyOpt;
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

sharpen::Optional<Test> GetTest(bool i)
{
    if (i)
    {
        return Test{};
    }
    return sharpen::EmptyOpt;
}

int main()
{
    std::printf("option test begin\n");
    {
        sharpen::Optional<int> opt = GetInt(false);
        std::puts("option<int> empty test\n");
        std::printf("opt<int> has value? %d\n",opt.Exist());
        assert(opt.Exist() == false);
        try
        {
            int a = opt.Get();
            assert(false);
        }
        catch(const sharpen::BadOptionError &e)
        {
            (void)e;
        }
        opt = GetInt(true);
        std::puts("option<int> value test\n");
        std::printf("opt<int> has value? %d\n",opt.Exist());
        assert(opt.Exist() == true);
        try
        {
            int a = opt.Get();
            assert(a == 1);
        }
        catch(const sharpen::BadOptionError &e)
        {
            (void)e;
            assert(false);
        }
    }
    {
        sharpen::Optional<Test> opt = GetTest(false);
        std::puts("option<Test> empty test\n");
        std::printf("opt has value? %d\n",opt.Exist());
        assert(opt.Exist() == false);
        try
        {
            Test &a = opt.Get();
            assert(false);
        }
        catch(const sharpen::BadOptionError &e)
        {
            (void)e;
        }
        std::puts("option<Test> value test\n");
        opt = GetTest(true);
        std::printf("opt has value? %d\n",opt.Exist());
        assert(opt.Exist() == true);
        try
        {
            Test &test = opt.Get();
        }
        catch(const sharpen::BadOptionError &e)
        {
            (void)e;
            assert(false);
        }
    }
    std::printf("option test pass\n");
    return 0;
}
