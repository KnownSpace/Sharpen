#include <cassert>
#include <cstdio>

#include <sharpen/AsyncLeaseBarrier.hpp>
#include <sharpen/AsyncOps.hpp>
#include <sharpen/AwaitOps.hpp>
#include <sharpen/DynamicWorkerGroup.hpp>
#include <sharpen/EventEngine.hpp>
#include <sharpen/FiberLocal.hpp>
#include <sharpen/FixedWorkerGroup.hpp>
#include <sharpen/SingleWorkerGroup.hpp>
#include <sharpen/TimerOps.hpp>
#include <sharpen/YieldOps.hpp>

#include <simpletest/TestRunner.hpp>

class AsyncTest : public simpletest::ITypenamedTest<AsyncTest> {
private:
    using Self = AsyncTest;

public:
    AsyncTest() noexcept = default;

    ~AsyncTest() noexcept = default;

    inline virtual simpletest::TestResult Run() noexcept {
        auto future{sharpen::Async([]() { return 1; })};
        return this->Assert(future->Await() == 1, "Await() return wrong anwser");
    }
};

class AwaitAllTest : public simpletest::ITypenamedTest<AwaitAllTest> {
private:
    using Self = AwaitAllTest;

public:
    AwaitAllTest() noexcept = default;

    ~AwaitAllTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        auto f1 = sharpen::Async([]() { return 1; });
        auto f2 = sharpen::Async([]() { return 1.1; });
        auto f3 = sharpen::Async([]() {
            // do nothing
        });
        std::int32_t r1;
        double r2;
        std::tie(r1, r2, std::ignore) = sharpen::AwaitAll(*f1, *f2, *f3);
        return this->Assert(r1 == 1 && r2 == 1.1, "AwaitAll() return wrong answer");
    }
};

class DelayTest : public simpletest::ITypenamedTest<DelayTest> {
private:
    using Self = DelayTest;

public:
    DelayTest() noexcept = default;

    ~DelayTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        auto t1{std::chrono::steady_clock::now()};
        sharpen::Delay(std::chrono::seconds(1));
        auto t2{std::chrono::steady_clock::now()};
        auto time{t2 - t1};
        std::uint32_t second{static_cast<std::uint32_t>(
            std::chrono::duration_cast<std::chrono::seconds>(time).count())};
        return this->Assert(second >= 1, "Delay() wait duration shorter than required");
    }
};

class AwaitAnyTest : public simpletest::ITypenamedTest<AwaitAnyTest> {
private:
    using Self = AwaitAnyTest;

public:
    AwaitAnyTest() noexcept = default;

    ~AwaitAnyTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        auto f1 = sharpen::Async([]() { return 0; });
        auto f2 = sharpen::Async([]() { sharpen::Delay(std::chrono::seconds(1)); });
        sharpen::AwaitAny(*f1, *f2);
        auto result = this->Assert(f2->IsPending(), "AwaitAnt() return wrong answer");
        f1->Await();
        f2->Await();
        return result;
    }
};

class ResetTest : public simpletest::ITypenamedTest<ResetTest> {
private:
    using Self = ResetTest;

public:
    ResetTest() noexcept = default;

    ~ResetTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        sharpen::AwaitableFuture<std::int32_t> future;
        sharpen::Launch([&future]() { future.Complete(1); });
        future.WaitAsync();
        future.Reset();
        sharpen::Launch([&future]() { future.Complete(2); });
        return this->Assert(future.Await() == 2, "Reset() doesn't reset status of future");
    }
};

class WorkerGroupTest : public simpletest::ITest {
private:
    using Self = WorkerGroupTest;
    using Base = simpletest::ITest;

    std::unique_ptr<sharpen::IWorkerGroup> workers_;
    std::size_t testCount_;

    inline static bool Check(std::vector<sharpen::AwaitableFuture<std::size_t>> &futures) {
        bool status{true};
        for (std::size_t i = 0; i != futures.size(); ++i) {
            if (futures[i].Await() != i) {
                status = false;
            }
        }
        return status;
    }

public:
    template<typename _Impl>
    WorkerGroupTest(_Impl *workers, std::size_t testCount) noexcept
        : Base(simpletest::GetReadableTypeName<_Impl>())
        , workers_(workers)
        , testCount_(testCount) {
    }

    ~WorkerGroupTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        std::vector<sharpen::AwaitableFuture<std::size_t>> futures;
        futures.resize(this->testCount_);
        for (std::size_t i = 0; i != this->testCount_; ++i) {
            this->workers_->Invoke(futures[i], [i]() {
                // long operation
                sharpen::YieldCycle();
                return i;
            });
        }
        auto result{this->Assert(this->Check(futures), "WorkerGroup return wrong answer")};
        this->workers_->Stop();
        this->workers_->Join();
        return result;
    }
};

class FiberLocalTest : public simpletest::ITypenamedTest<FiberLocalTest> {
private:
    using Self = FiberLocalTest;

public:
    FiberLocalTest() noexcept = default;

    ~FiberLocalTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        sharpen::FiberLocal<std::uint64_t> local;
        local.New(static_cast<std::uint64_t>(1));
        auto result =
            sharpen::Async([&local]() {
                local.New(static_cast<std::uint64_t>(0));
                return Self::Assert(*local.Lookup() == 0, "*local.Lookup() should = 0,but not");
            })->Await();
        if (result.Fail()) {
            return result;
        }
        return this->Assert(*local.Lookup() == 1, "*local.Lookup() should = 1,but not");
    }
};

static int Test() {
    constexpr std::size_t workerGroupJobs{256 * 1024};
    simpletest::TestRunner runner;
    runner.Register<AsyncTest>();
    runner.Register<AwaitAllTest>();
    runner.Register<AwaitAnyTest>();
    runner.Register<WorkerGroupTest>(
        new (std::nothrow) sharpen::FixedWorkerGroup{*sharpen::GetLocalSchedulerPtr()},
        workerGroupJobs);
    runner.Register<WorkerGroupTest>(
        new (std::nothrow) sharpen::SingleWorkerGroup{*sharpen::GetLocalSchedulerPtr()},
        workerGroupJobs);
    runner.Register<WorkerGroupTest>(
        new (std::nothrow) sharpen::DynamicWorkerGroup{*sharpen::GetLocalSchedulerPtr()},
        workerGroupJobs);
    runner.Register<FiberLocalTest>();
    return runner.Run();
}

int main() {
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupEngine();
    return engine.StartupWithCode(&Test);
}
