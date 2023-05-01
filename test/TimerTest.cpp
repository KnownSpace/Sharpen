#include <cassert>
#include <cstdio>

#include <sharpen/AsyncOps.hpp>
#include <sharpen/EventEngine.hpp>
#include <sharpen/StopWatcher.hpp>
#include <sharpen/TimerOps.hpp>

#include <simpletest/TestRunner.hpp>

class CancelTest : public simpletest::ITypenamedTest<CancelTest> {
private:
    using Self = CancelTest;

public:
    CancelTest() noexcept = default;

    ~CancelTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        sharpen::TimerPtr timer = sharpen::MakeTimer(sharpen::GetLocalLoopGroup());
        sharpen::AwaitableFuture<bool> future;
        sharpen::StopWatcher sw;
        sw.Begin();
        timer->WaitAsync(future, std::chrono::seconds(3));
        timer->Cancel();
        future.Await();
        sw.Stop();
        return this->Assert(sw.Compute() < 3 * CLOCKS_PER_SEC,
                            "Wait time should < 3sec,but it not");
    }
};

class AwaitForTest : public simpletest::ITypenamedTest<AwaitForTest> {
private:
    using Self = AwaitForTest;

public:
    AwaitForTest() noexcept = default;

    ~AwaitForTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        sharpen::UniquedTimerRef timer{sharpen::GetUniquedTimerRef()};
        sharpen::AwaitableFuture<void> future;
        sharpen::AwaitForResult result{
            sharpen::AwaitFor(future, timer.Timer(), std::chrono::seconds(1))};
        return this->Assert(result == sharpen::AwaitForResult::Timeout,
                            "result should == Timeout,but it not");
    }
};

class AwaitForCompletedTest : public simpletest::ITypenamedTest<AwaitForCompletedTest> {
private:
    using Self = AwaitForCompletedTest;

public:
    AwaitForCompletedTest() noexcept = default;

    ~AwaitForCompletedTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        sharpen::UniquedTimerRef timer{sharpen::GetUniquedTimerRef()};
        sharpen::AwaitableFuture<void> future;
        sharpen::Launch([&future]() {
            sharpen::Delay(std::chrono::milliseconds(500));
            future.Complete();
        });
        sharpen::AwaitForResult result{
            sharpen::AwaitFor(future, timer.Timer(), std::chrono::seconds(1))};
        return this->Assert(result == sharpen::AwaitForResult::CompletedOrError,
                            "result should == CompletedOrError,but it not");
    }
};

static int Test() {
    simpletest::TestRunner runner;
    runner.Register<CancelTest>();
    runner.Register<AwaitForTest>();
    runner.Register<AwaitForCompletedTest>();
    return runner.Run();
}

int main(int argc, char const *argv[]) {
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    return engine.StartupWithCode(&Test);
}
