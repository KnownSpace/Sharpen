#include <cassert>
#include <cstdio>

#include <sharpen/AsyncOps.hpp>
#include <sharpen/EventEngine.hpp>
#include <sharpen/AsyncLeaseLock.hpp>
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

class LeaseLockTest : public simpletest::ITypenamedTest<LeaseLockTest> {
private:
    using Self = LeaseLockTest;

public:
    LeaseLockTest() noexcept = default;

    ~LeaseLockTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        sharpen::AsyncLeaseLock lock{std::chrono::seconds{1}};
        auto first{std::chrono::steady_clock::now()};
        sharpen::AwaitableFuture<void> future;
        sharpen::Launch([&lock,&future]() {
            std::unique_lock<sharpen::AsyncLeaseLock> _lock{lock};
            future.Complete();
        });
        future.Await();
        {
            std::unique_lock<sharpen::AsyncLeaseLock> _lock{lock};
            auto second{std::chrono::steady_clock::now()};
            return this->Assert(std::chrono::duration_cast<std::chrono::seconds>(second - first).count() == 0,"should not use 1 sec");
        }
    }
};

class LeaseLockTimeoutTest:public simpletest::ITypenamedTest<LeaseLockTimeoutTest>
{
private:
    using Self = LeaseLockTimeoutTest;

public:

    LeaseLockTimeoutTest() noexcept = default;

    ~LeaseLockTimeoutTest() noexcept = default;

    inline const Self &Const() const noexcept
    {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept
    {
        sharpen::AsyncLeaseLock lock{std::chrono::seconds{1}};
        auto first{std::chrono::steady_clock::now()};
        sharpen::AwaitableFuture<void> future;
        sharpen::Launch([&lock,&future]() {
            lock.LockAsync();
            future.Complete();
        });
        future.Await();
        {
            std::unique_lock<sharpen::AsyncLeaseLock> _lock{lock};
            auto second{std::chrono::steady_clock::now()};
            auto time{std::chrono::duration_cast<std::chrono::milliseconds>(second - first)};
            return this->Assert(time.count() >= 1,"should use 1 sec or more");
        }
    }
};

static int Test() {
    simpletest::TestRunner runner;
    runner.Register<CancelTest>();
    runner.Register<AwaitForTest>();
    runner.Register<AwaitForCompletedTest>();
    runner.Register<LeaseLockTest>();
    runner.Register<LeaseLockTimeoutTest>();
    return runner.Run();
}

int main(int argc, char const *argv[]) {
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    return engine.StartupWithCode(&Test);
}
