#include <cinttypes>
#include <csignal>
#include <cstdio>

#include <sharpen/AsyncOps.hpp>
#include <sharpen/EventEngine.hpp>
#include <sharpen/ISignalChannel.hpp>
#include <sharpen/TimerOps.hpp>

#include <simpletest/TestRunner.hpp>

class SignalTest : public simpletest::ITypenamedTest<SignalTest> {
private:
    using Self = SignalTest;

public:
    SignalTest() noexcept = default;

    ~SignalTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        std::int32_t sig{SIGINT};
        sharpen::SignalChannelPtr channel{sharpen::OpenSignalChannel(&sig, 1)};
        channel->Register(sharpen::GetLocalLoopGroup());
        sharpen::SignalBuffer signals{1};
        sharpen::Launch([sig]() {
            sharpen::Delay(std::chrono::seconds(3));
            std::raise(sig);
        });
        std::size_t size{channel->ReadAsync(signals)};
        if (size != 1) {
            return this->Fail("size could == 1,but it not");
        }
        return this->Assert(signals.PopSignal() == sig, "sig could == SIGINT,but it not");
    }
};

class CloseTest : public simpletest::ITypenamedTest<CloseTest> {
private:
    using Self = CloseTest;

public:
    CloseTest() noexcept = default;

    ~CloseTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        std::int32_t sig{SIGINT};
        sharpen::SignalChannelPtr channel{sharpen::OpenSignalChannel(&sig, 1)};
        channel->Register(sharpen::GetLocalLoopGroup());
        sharpen::SignalBuffer signals{1};
        sharpen::Launch([channel]() {
            sharpen::Delay(std::chrono::seconds(3));
            channel->Close();
        });
        std::size_t size{channel->ReadAsync(signals)};
        return this->Assert(size == 0, "size could == 0,but it not");
    }
};

static int Test() {
    simpletest::TestRunner runner;
    runner.Register<SignalTest>();
    runner.Register<CloseTest>();
    return runner.Run();
}

int main(int argc, char const *argv[]) {
    sharpen::EventEngine &engine{sharpen::EventEngine::SetupSingleThreadEngine()};
    return engine.StartupWithCode(&Test);
}