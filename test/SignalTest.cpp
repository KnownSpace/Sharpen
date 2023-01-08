#include <cstdio>
#include <csignal>
#include <cinttypes>

#include <sharpen/EventEngine.hpp>
#include <sharpen/ISignalChannel.hpp>
#include <sharpen/TimerOps.hpp>
#include <sharpen/AsyncOps.hpp>

#include <simpletest/TestRunner.hpp>

class SignalTest:public simpletest::ITypenamedTest<SignalTest>
{
private:
    using Self = SignalTest;

public:

    SignalTest() noexcept = default;

    ~SignalTest() noexcept = default;

    inline const Self &Const() const noexcept
    {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept
    {
        std::int32_t sig{SIGINT};
        sharpen::SignalChannelPtr channel{sharpen::OpenSignalChannel(&sig,1)};
        channel->Register(sharpen::GetLocalLoopGroup());
        sharpen::SignalBuffer signals{1};
        sharpen::Launch([](){
            sharpen::Delay(std::chrono::seconds(3));
            std::raise(SIGINT);
        });
        std::size_t size{channel->ReadAsync(signals)};
        if(size != 1)
        {
            return this->Fail("size could == 1,but it not");
        }
        return this->Assert(sig == SIGINT,"sig could == SIGINT,but it not");
    }
};

static int Startup()
{
    simpletest::TestRunner runner;
    runner.Register<SignalTest>();
    return runner.Run();
}

int main(int argc, char const *argv[])
{
    sharpen::EventEngine &engine{sharpen::EventEngine::SetupSingleThreadEngine()};
    return engine.StartupWithCode(&Startup);
}