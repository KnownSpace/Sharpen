#include <cassert>
#include <cstdio>
#include <cstring>

#include <sharpen/BufferOps.hpp>

#include <simpletest/TestRunner.hpp>

class Base64EncodingTest : public simpletest::ITypenamedTest<Base64EncodingTest>
{
private:
    using Self = Base64EncodingTest;

public:
    Base64EncodingTest() noexcept = default;

    ~Base64EncodingTest() noexcept = default;

    inline const Self &Const() const noexcept
    {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept
    {
        char str[] = "ABC";
        char buf[5] = {0};
        sharpen::Base64Encode(buf, sizeof(buf) - 1, str, sizeof(str) - 1);
        return this->Assert(!std::strcmp("QUJD", buf), "Base64Encode(\"ABC\") != \"QUJD\"");
    }
};

class Base64DecodingTest : public simpletest::ITypenamedTest<Base64DecodingTest>
{
private:
    using Self = Base64DecodingTest;

public:
    Base64DecodingTest() noexcept = default;

    ~Base64DecodingTest() noexcept = default;

    inline const Self &Const() const noexcept
    {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept
    {
        char buf[] = "QUJD";
        char str[4] = {0};
        sharpen::Base64Decode(str, sizeof(str) - 1, buf, sizeof(buf) - 1);
        return this->Assert(!std::strcmp("ABC", str), "Base64Decode(\"QUJD\") != \"ABC\"");
    }
};

int main(int argc, char const *argv[])
{
    simpletest::TestRunner runner;
    runner.Register<Base64EncodingTest>();
    runner.Register<Base64DecodingTest>();
    return runner.Run();
}
