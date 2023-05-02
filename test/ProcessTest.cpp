#include <cstdio>

#include <sharpen/EventEngine.hpp>
#include <sharpen/Process.hpp>

#include <simpletest/TestRunner.hpp>

class RedirectTest : public simpletest::ITypenamedTest<RedirectTest> {
private:
    using Self = RedirectTest;

public:
    RedirectTest() noexcept = default;

    ~RedirectTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
#ifdef SHARPEN_IS_NIX
        std::vector<std::string> args;
        args.emplace_back("Hello World");
        sharpen::Process process{"/usr/bin/echo", args.begin(), args.end()};
        sharpen::InputPipeChannelPtr pOut{process.RedirectStdout()};
        pOut->Register(sharpen::GetLocalLoopGroup());
        process.Start();
        process.Join();
        sharpen::ByteBuffer buf{4096};
        std::size_t sz{pOut->ReadAsync(buf)};
        return this->Assert(sz != 0 && std::strncmp("Hello World", buf.Data(), 11) == 0,
                            "Output should equal with \"Hello World\", but it not");
#else
        return this->Success();
#endif
    }
};

int Test() {
    simpletest::TestRunner runner;
    runner.Register<RedirectTest>();
    return runner.Run();
}

int main(int argc, char const *argv[]) {
    sharpen::EventEngine &engine{sharpen::EventEngine::SetupSingleThreadEngine()};
    return engine.StartupWithCode(&Test);
}