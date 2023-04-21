#include <cassert>
#include <cstdio>

#include <sharpen/BufferOps.hpp>

#include <simpletest/TestRunner.hpp>

class BufferEqualTest : public simpletest::ITypenamedTest<BufferEqualTest>
{
private:
    using Self = BufferEqualTest;

public:
    BufferEqualTest() noexcept = default;

    ~BufferEqualTest() noexcept = default;

    inline const Self &Const() const noexcept
    {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept
    {
        char lhs[] = "abc";
        char rhs[] = "abc";
        std::int32_t r{sharpen::BufferCompare(lhs, sizeof(lhs), rhs, sizeof(rhs))};
        return this->Assert(r == 0, "\"abc\" != \"abc\"");
    }
};

class BufferLargerTest : public simpletest::ITypenamedTest<BufferLargerTest>
{
private:
    using Self = BufferLargerTest;

public:
    BufferLargerTest() noexcept = default;

    ~BufferLargerTest() noexcept = default;

    inline const Self &Const() const noexcept
    {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept
    {
        char lhs[] = "abd";
        char rhs[] = "abc";
        std::int32_t r{sharpen::BufferCompare(lhs, sizeof(lhs), rhs, sizeof(rhs))};
        return this->Assert(r == 1, "\"abd\" should > \"abc\",but it not");
    }
};

class BufferLongerTest : public simpletest::ITypenamedTest<BufferLongerTest>
{
private:
    using Self = BufferLongerTest;

public:
    BufferLongerTest() noexcept = default;

    ~BufferLongerTest() noexcept = default;

    inline const Self &Const() const noexcept
    {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept
    {
        char lhs[] = "abcd";
        char rhs[] = "abc";
        std::int32_t r{sharpen::BufferCompare(lhs, sizeof(lhs), rhs, sizeof(rhs))};
        return this->Assert(r == 1, "\"abcd\" should > \"abc\",but it not");
    }
};

int main(int argc, char const *argv[])
{
    simpletest::TestRunner runner;
    runner.Register<BufferEqualTest>();
    runner.Register<BufferLargerTest>();
    runner.Register<BufferLongerTest>();
    return runner.Run();
}
