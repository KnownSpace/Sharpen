#include <cassert>
#include <cstdio>

#include <sharpen/BufferOps.hpp>

#include <simpletest/TestRunner.hpp>

class Crc16Test : public simpletest::ITypenamedTest<Crc16Test> {
private:
    using Self = Crc16Test;

public:
    Crc16Test() noexcept = default;

    ~Crc16Test() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        char buf[] = "123456789";
        std::uint32_t checksum = sharpen::Crc16(buf, sizeof(buf) - 1);
        return this->Assert(checksum == 0x4B37, "Crc16(\"123456789\") should == 0x4B37,but it not");
    }
};

class Adler32Test : public simpletest::ITypenamedTest<Adler32Test> {
private:
    using Self = Adler32Test;

public:
    Adler32Test() noexcept = default;

    ~Adler32Test() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        char buf[] = "123456789";
        std::uint32_t checksum = sharpen::Adler32(buf, sizeof(buf) - 1);
        return this->Assert(checksum == 0x91E01DE,
                            "Adler32(\"123456789\") should == 0x91E01DE,but it not");
    }
};

class Crc32Tester : public simpletest::ITypenamedTest<Crc32Tester> {
private:
    using Self = Crc32Tester;

public:
    Crc32Tester() noexcept = default;

    ~Crc32Tester() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        char buf[] = "123456789";
        std::uint32_t checksum = sharpen::Crc32(buf, sizeof(buf) - 1);
        return this->Assert(checksum == 0xCBF43926,
                            "Crc32(\"123456789\") should == 0xCBF43926,but it not");
    }
};

int main(int argc, char const *argv[]) {
    simpletest::TestRunner runner;
    runner.Register<Crc16Test>();
    runner.Register<Adler32Test>();
    runner.Register<Crc32Tester>();
    return runner.Run();
}
