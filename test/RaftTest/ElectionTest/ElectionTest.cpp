#include <common/LogStep.hpp>
#include <common/RaftStep.hpp>
#include <common/RaftTool.hpp>
#include <sharpen/AsyncOps.hpp>
#include <sharpen/BinarySerializable.hpp>
#include <sharpen/BufferWriter.hpp>
#include <sharpen/DebugTools.hpp>
#include <sharpen/EventEngine.hpp>
#include <sharpen/GenericMailParserFactory.hpp>
#include <sharpen/IMailReceiver.hpp>
#include <sharpen/IpTcpActorBuilder.hpp>
#include <sharpen/IpTcpStreamFactory.hpp>
#include <sharpen/MultiRaftMailBuilder.hpp>
#include <sharpen/MultiRaftMailExtractor.hpp>
#include <sharpen/RaftConsensus.hpp>
#include <sharpen/RaftLogAccesser.hpp>
#include <sharpen/RaftMailBuilder.hpp>
#include <sharpen/RaftMailExtractor.hpp>
#include <sharpen/SimpleHostPipeline.hpp>
#include <sharpen/SingleWorkerGroup.hpp>
#include <sharpen/TcpActor.hpp>
#include <sharpen/TcpHost.hpp>
#include <sharpen/TimerOps.hpp>
#include <sharpen/WalLogStorage.hpp>
#include <sharpen/WalStatusMap.hpp>
#include <simpletest/TestRunner.hpp>
#include <cinttypes>
#include <sstream>
#include <vector>



static const std::uint32_t magicNumber{0x2333};

static const std::uint16_t beginPort{10801};

static const std::uint16_t endPort{10803};

static constexpr std::size_t spitBrainTestCount{60};

static std::shared_ptr<sharpen::IConsensus> CreateRaft(std::uint16_t port) {
    sharpen::RaftOption raftOpt;
    raftOpt.SetBatchSize(16);
    raftOpt.SetLearner(false);
    raftOpt.SetPrevote(false);
    auto raft{CreateRaft(port, magicNumber, nullptr, nullptr, raftOpt, false)};
    raft->ConfiguratePeers(
        &ConfigPeers, port, beginPort, endPort, &raft->GetReceiver(), magicNumber, false);
    return raft;
}

static std::shared_ptr<sharpen::IConsensus> CreatePrevoteRaft(std::uint16_t port) {
    sharpen::RaftOption raftOpt;
    raftOpt.SetBatchSize(16);
    raftOpt.SetLearner(false);
    raftOpt.SetPrevote(true);
    auto raft{CreateRaft(port, magicNumber, nullptr, nullptr, raftOpt, false)};
    raft->ConfiguratePeers(
        &ConfigPeers, port, beginPort, endPort, &raft->GetReceiver(), magicNumber, false);
    return raft;
}

static std::shared_ptr<sharpen::IConsensus> CreateBalanceRaft(
    std::uint16_t port, std::shared_ptr<sharpen::RaftLeaderCounter> counter) {
    sharpen::RaftOption raftOpt;
    raftOpt.SetBatchSize(16);
    raftOpt.SetLearner(false);
    raftOpt.SetPrevote(false);
    auto raft{CreateRaft(port, magicNumber, nullptr, std::move(counter), raftOpt, false)};
    raft->ConfiguratePeers(
        &ConfigPeers, port, beginPort, endPort, &raft->GetReceiver(), magicNumber, false);
    return raft;
}

std::unique_ptr<sharpen::IHostPipeline> ConfigPipeline(std::shared_ptr<sharpen::IConsensus> raft) {
    std::unique_ptr<sharpen::IHostPipeline> pipe{new (std::nothrow) sharpen::SimpleHostPipeline{}};
    pipe->Register<LogStep>();
    pipe->Register<RaftStep>(magicNumber, std::move(raft));
    return pipe;
}

std::unique_ptr<sharpen::TcpHost> CreateHost(std::uint16_t port,
                                             std::shared_ptr<sharpen::IConsensus> raft) {
    sharpen::IpEndPoint endPoint;
    endPoint.SetAddrByString("127.0.0.1");
    endPoint.SetPort(port);
    sharpen::IpTcpStreamFactory streamFactory{endPoint};
    std::unique_ptr<sharpen::TcpHost> host{new (std::nothrow) sharpen::TcpHost{streamFactory}};
    if (!host) {
        throw std::bad_alloc{};
    }
    host->ConfiguratePipeline(&ConfigPipeline, std::move(raft));
    return host;
}

class BasicElectionTest : public simpletest::ITypenamedTest<BasicElectionTest> {
private:
    using Self = BasicElectionTest;

public:
    BasicElectionTest() noexcept = default;

    ~BasicElectionTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        std::vector<std::shared_ptr<sharpen::IConsensus>> rafts;
        rafts.reserve(3);
        std::vector<std::unique_ptr<sharpen::IHost>> hosts;
        hosts.reserve(3);
        std::vector<sharpen::AwaitableFuturePtr<void>> process;
        process.reserve(3);
        for (std::uint16_t i = beginPort; i != endPort + 1; ++i) {
            auto raft{CreateRaft(i)};
            rafts.emplace_back(raft);
            auto host{CreateHost(i, raft)};
            hosts.emplace_back(std::move(host));
        }
        for (auto begin = hosts.begin(), end = hosts.end(); begin != end; ++begin) {
            sharpen::IHost *host{begin->get()};
            auto future{sharpen::Async([host]() { host->Run(); })};
            process.emplace_back(std::move(future));
        }
        auto primary{rafts[0].get()};
        primary->Advance();
        primary->WaitNextConsensus();
        bool writable{primary->Writable()};
        // close all hosts
        for (auto begin = rafts.begin(), end = rafts.end(); begin != end; ++begin) {
            auto raft{begin->get()};
            raft->ReleasePeers();
        }
        for (auto begin = hosts.begin(), end = hosts.end(); begin != end; ++begin) {
            auto host{begin->get()};
            host->Stop();
        }
        for (auto begin = process.begin(), end = process.end(); begin != end; ++begin) {
            auto future{begin->get()};
            future->WaitAsync();
        }
        // remove files
        for (std::uint16_t i = beginPort; i != endPort + 1; ++i) {
            RemoveLogStorage(i);
            RemoveStatusMap(i);
        }
        return this->Assert(writable, "should be writable");
    }
};

class FaultElectionTest : public simpletest::ITypenamedTest<FaultElectionTest> {
private:
    using Self = FaultElectionTest;

public:
    FaultElectionTest() noexcept = default;

    ~FaultElectionTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        std::vector<std::shared_ptr<sharpen::IConsensus>> rafts;
        rafts.reserve(2);
        std::vector<std::unique_ptr<sharpen::IHost>> hosts;
        hosts.reserve(2);
        std::vector<sharpen::AwaitableFuturePtr<void>> process;
        process.reserve(2);
        for (std::uint16_t i = beginPort; i != endPort; ++i) {
            auto raft{CreateRaft(i)};
            rafts.emplace_back(raft);
            auto host{CreateHost(i, raft)};
            hosts.emplace_back(std::move(host));
        }
        for (auto begin = hosts.begin(), end = hosts.end(); begin != end; ++begin) {
            sharpen::IHost *host{begin->get()};
            auto future{sharpen::Async([host]() { host->Run(); })};
            process.emplace_back(std::move(future));
        }
        auto primary{rafts[0].get()};
        primary->Advance();
        primary->WaitNextConsensus();
        bool writable{primary->Writable()};
        // close all hosts
        for (auto begin = rafts.begin(), end = rafts.end(); begin != end; ++begin) {
            auto raft{begin->get()};
            raft->ReleasePeers();
        }
        for (auto begin = hosts.begin(), end = hosts.end(); begin != end; ++begin) {
            auto host{begin->get()};
            host->Stop();
        }
        for (auto begin = process.begin(), end = process.end(); begin != end; ++begin) {
            auto future{begin->get()};
            future->WaitAsync();
        }
        // remove files
        for (std::uint16_t i = beginPort; i != endPort; ++i) {
            RemoveLogStorage(i);
            RemoveStatusMap(i);
        }
        return this->Assert(writable, "should be writable");
    }
};

class SplitBrainTest : public simpletest::ITypenamedTest<SplitBrainTest> {
private:
    using Self = SplitBrainTest;

public:
    SplitBrainTest() noexcept = default;

    ~SplitBrainTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        std::vector<std::shared_ptr<sharpen::IConsensus>> rafts;
        rafts.reserve(3);
        std::vector<std::unique_ptr<sharpen::IHost>> hosts;
        hosts.reserve(3);
        std::vector<sharpen::AwaitableFuturePtr<void>> process;
        process.reserve(3);
        for (std::uint16_t i = beginPort; i != endPort + 1; ++i) {
            auto raft{CreateRaft(i)};
            rafts.emplace_back(raft);
            auto host{CreateHost(i, raft)};
            hosts.emplace_back(std::move(host));
        }
        for (auto begin = hosts.begin(), end = hosts.end(); begin != end; ++begin) {
            sharpen::IHost *host{begin->get()};
            auto future{sharpen::Async([host]() { host->Run(); })};
            process.emplace_back(std::move(future));
        }
        std::size_t count{0};
        // raise elections
        for (std::size_t i = 0; i != spitBrainTestCount; ++i) {
            for (auto begin = rafts.begin(), end = rafts.end(); begin != end; ++begin) {
                auto raft{begin->get()};
                raft->Advance();
            }
            sharpen::Delay(std::chrono::seconds(1));
            for (auto begin = rafts.begin(), end = rafts.end(); begin != end; ++begin) {
                auto raft{begin->get()};
                sharpen::SyncPrintf("Raft %zu Writable %d Epoch %" PRIu64 "\n",
                                    begin - rafts.begin(),
                                    raft->Writable(),
                                    raft->GetEpoch());
                if (raft->Writable()) {
                    count += 1;
                }
            }
        }
        // close all hosts
        for (auto begin = rafts.begin(), end = rafts.end(); begin != end; ++begin) {
            auto raft{begin->get()};
            raft->ReleasePeers();
        }
        for (auto begin = hosts.begin(), end = hosts.end(); begin != end; ++begin) {
            auto host{begin->get()};
            host->Stop();
        }
        for (auto begin = process.begin(), end = process.end(); begin != end; ++begin) {
            auto future{begin->get()};
            future->WaitAsync();
        }
        sharpen::SyncPrintf("Leader Count %zu/%zu\n", count, spitBrainTestCount);
        // remove files
        for (std::uint16_t i = beginPort; i != endPort + 1; ++i) {
            RemoveLogStorage(i);
            RemoveStatusMap(i);
        }
        return this->Assert(count / spitBrainTestCount <= 1, "split brain");
    }
};

class PrevoteElectionTest : public simpletest::ITypenamedTest<PrevoteElectionTest> {
private:
    using Self = PrevoteElectionTest;

public:
    PrevoteElectionTest() noexcept = default;

    ~PrevoteElectionTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        std::vector<std::shared_ptr<sharpen::IConsensus>> rafts;
        rafts.reserve(3);
        std::vector<std::unique_ptr<sharpen::IHost>> hosts;
        hosts.reserve(3);
        std::vector<sharpen::AwaitableFuturePtr<void>> process;
        process.reserve(3);
        for (std::uint16_t i = beginPort; i != endPort + 1; ++i) {
            auto raft{CreatePrevoteRaft(i)};
            rafts.emplace_back(raft);
            auto host{CreateHost(i, raft)};
            hosts.emplace_back(std::move(host));
        }
        for (auto begin = hosts.begin(), end = hosts.end(); begin != end; ++begin) {
            sharpen::IHost *host{begin->get()};
            auto future{sharpen::Async([host]() { host->Run(); })};
            process.emplace_back(std::move(future));
        }
        auto primary{rafts[0].get()};
        primary->Advance();
        primary->WaitNextConsensus();
        bool writable{primary->Writable()};
        // close all hosts
        for (auto begin = rafts.begin(), end = rafts.end(); begin != end; ++begin) {
            auto raft{begin->get()};
            raft->ReleasePeers();
        }
        for (auto begin = hosts.begin(), end = hosts.end(); begin != end; ++begin) {
            auto host{begin->get()};
            host->Stop();
        }
        for (auto begin = process.begin(), end = process.end(); begin != end; ++begin) {
            auto future{begin->get()};
            future->WaitAsync();
        }
        // remove files
        for (std::uint16_t i = beginPort; i != endPort + 1; ++i) {
            RemoveLogStorage(i);
            RemoveStatusMap(i);
        }
        return this->Assert(writable, "should be writable");
    }
};

class FaultPrevoteTest : public simpletest::ITypenamedTest<FaultPrevoteTest> {
private:
    using Self = FaultPrevoteTest;

public:
    FaultPrevoteTest() noexcept = default;

    ~FaultPrevoteTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        std::vector<std::shared_ptr<sharpen::IConsensus>> rafts;
        rafts.reserve(2);
        std::vector<std::unique_ptr<sharpen::IHost>> hosts;
        hosts.reserve(2);
        std::vector<sharpen::AwaitableFuturePtr<void>> process;
        process.reserve(2);
        for (std::uint16_t i = beginPort; i != endPort; ++i) {
            auto raft{CreatePrevoteRaft(i)};
            rafts.emplace_back(raft);
            auto host{CreateHost(i, raft)};
            hosts.emplace_back(std::move(host));
        }
        for (auto begin = hosts.begin(), end = hosts.end(); begin != end; ++begin) {
            sharpen::IHost *host{begin->get()};
            auto future{sharpen::Async([host]() { host->Run(); })};
            process.emplace_back(std::move(future));
        }
        auto primary{rafts[0].get()};
        primary->Advance();
        primary->WaitNextConsensus();
        bool writable{primary->Writable()};
        // close all hosts
        for (auto begin = rafts.begin(), end = rafts.end(); begin != end; ++begin) {
            auto raft{begin->get()};
            raft->ReleasePeers();
        }
        for (auto begin = hosts.begin(), end = hosts.end(); begin != end; ++begin) {
            auto host{begin->get()};
            host->Stop();
        }
        for (auto begin = process.begin(), end = process.end(); begin != end; ++begin) {
            auto future{begin->get()};
            future->WaitAsync();
        }
        // remove files
        for (std::uint16_t i = beginPort; i != endPort; ++i) {
            RemoveLogStorage(i);
            RemoveStatusMap(i);
        }
        return this->Assert(writable, "should be writable");
    }
};

class FailedPrevoteTest : public simpletest::ITypenamedTest<FailedPrevoteTest> {
private:
    using Self = FailedPrevoteTest;

public:
    FailedPrevoteTest() noexcept = default;

    ~FailedPrevoteTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        std::vector<std::shared_ptr<sharpen::IConsensus>> rafts;
        rafts.reserve(3);
        std::vector<std::unique_ptr<sharpen::IHost>> hosts;
        hosts.reserve(3);
        std::vector<sharpen::AwaitableFuturePtr<void>> process;
        process.reserve(3);
        for (std::uint16_t i = beginPort; i != endPort + 1; ++i) {
            auto raft{CreatePrevoteRaft(i)};
            rafts.emplace_back(raft);
            sharpen::LogBatch logs;
            logs.Append(sharpen::ByteBuffer{"log", 3});
            if (i != beginPort) {
                raft->Write(logs);
            }
            auto host{CreateHost(i, raft)};
            hosts.emplace_back(std::move(host));
        }
        for (auto begin = hosts.begin(), end = hosts.end(); begin != end; ++begin) {
            sharpen::IHost *host{begin->get()};
            auto future{sharpen::Async([host]() { host->Run(); })};
            process.emplace_back(std::move(future));
        }
        auto primary{rafts[0].get()};
        primary->Advance();
        sharpen::Future<sharpen::ConsensusResult> future;
        primary->WaitNextConsensus(future);
        // close all hosts
        for (auto begin = rafts.begin(), end = rafts.end(); begin != end; ++begin) {
            auto raft{begin->get()};
            raft->ReleasePeers();
        }
        for (auto begin = hosts.begin(), end = hosts.end(); begin != end; ++begin) {
            auto host{begin->get()};
            host->Stop();
        }
        for (auto begin = process.begin(), end = process.end(); begin != end; ++begin) {
            auto future{begin->get()};
            future->WaitAsync();
        }
        // remove files
        for (std::uint16_t i = beginPort; i != endPort + 1; ++i) {
            RemoveLogStorage(i);
            RemoveStatusMap(i);
        }
        return this->Assert(future.IsPending(), "should be pending");
    }
};

class BasicLeaderBalanceTest : public simpletest::ITypenamedTest<BasicLeaderBalanceTest> {
private:
    using Self = BasicLeaderBalanceTest;

public:
    BasicLeaderBalanceTest() noexcept = default;

    ~BasicLeaderBalanceTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        std::vector<std::shared_ptr<sharpen::IConsensus>> rafts;
        rafts.reserve(3);
        std::vector<std::unique_ptr<sharpen::IHost>> hosts;
        hosts.reserve(3);
        std::vector<sharpen::AwaitableFuturePtr<void>> process;
        process.reserve(3);
        std::vector<std::shared_ptr<sharpen::RaftLeaderCounter>> counters;
        counters.reserve(3);
        for (std::size_t i = 0; i != 3; ++i) {
            counters.emplace_back(std::make_shared<sharpen::RaftLeaderCounter>());
        }
        for (std::uint16_t i = beginPort; i != endPort + 1; ++i) {
            auto raft{CreateBalanceRaft(i, counters[i - beginPort])};
            rafts.emplace_back(raft);
            auto host{CreateHost(i, raft)};
            hosts.emplace_back(std::move(host));
        }
        for (auto begin = hosts.begin(), end = hosts.end(); begin != end; ++begin) {
            sharpen::IHost *host{begin->get()};
            auto future{sharpen::Async([host]() { host->Run(); })};
            process.emplace_back(std::move(future));
        }
        auto primary{rafts[0].get()};
        auto primaryCount{counters[0].get()};
        // increase counter
        primaryCount->TryComeToPower(0);
        sharpen::AwaitableFuture<sharpen::ConsensusResult> future;
        primary->Advance();
        primary->WaitNextConsensus(future);
        sharpen::Delay(std::chrono::seconds{1});
        bool writable{primary->Writable()};
        if (writable) {
            // close all hosts
            for (auto begin = rafts.begin(), end = rafts.end(); begin != end; ++begin) {
                auto raft{begin->get()};
                raft->ReleasePeers();
            }
            for (auto begin = hosts.begin(), end = hosts.end(); begin != end; ++begin) {
                auto host{begin->get()};
                host->Stop();
            }
            for (auto begin = process.begin(), end = process.end(); begin != end; ++begin) {
                auto future{begin->get()};
                future->WaitAsync();
            }
            // remove files
            for (std::uint16_t i = beginPort; i != endPort + 1; ++i) {
                RemoveLogStorage(i);
                RemoveStatusMap(i);
            }
            return this->Fail("should not be writable");
        }
        primaryCount->Abdicate();
        primary->Advance();
        future.Await();
        writable = primary->Writable();
        // close all hosts
        for (auto begin = rafts.begin(), end = rafts.end(); begin != end; ++begin) {
            auto raft{begin->get()};
            raft->ReleasePeers();
        }
        for (auto begin = hosts.begin(), end = hosts.end(); begin != end; ++begin) {
            auto host{begin->get()};
            host->Stop();
        }
        for (auto begin = process.begin(), end = process.end(); begin != end; ++begin) {
            auto future{begin->get()};
            future->WaitAsync();
        }
        // remove files
        for (std::uint16_t i = beginPort; i != endPort + 1; ++i) {
            RemoveLogStorage(i);
            RemoveStatusMap(i);
        }
        return this->Assert(writable, "should be writable");
    }
};

class FaultBalanceTest : public simpletest::ITypenamedTest<FaultBalanceTest> {
private:
    using Self = FaultBalanceTest;

public:
    FaultBalanceTest() noexcept = default;

    ~FaultBalanceTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        std::vector<std::shared_ptr<sharpen::IConsensus>> rafts;
        rafts.reserve(2);
        std::vector<std::unique_ptr<sharpen::IHost>> hosts;
        hosts.reserve(2);
        std::vector<sharpen::AwaitableFuturePtr<void>> process;
        process.reserve(2);
        std::vector<std::shared_ptr<sharpen::RaftLeaderCounter>> counters;
        counters.reserve(2);
        for (std::size_t i = 0; i != 2; ++i) {
            counters.emplace_back(std::make_shared<sharpen::RaftLeaderCounter>());
        }
        for (std::uint16_t i = beginPort; i != endPort; ++i) {
            auto raft{CreateBalanceRaft(i, counters[i - beginPort])};
            rafts.emplace_back(raft);
            auto host{CreateHost(i, raft)};
            hosts.emplace_back(std::move(host));
        }
        for (auto begin = hosts.begin(), end = hosts.end(); begin != end; ++begin) {
            sharpen::IHost *host{begin->get()};
            auto future{sharpen::Async([host]() { host->Run(); })};
            process.emplace_back(std::move(future));
        }
        auto primary{rafts[0].get()};
        auto primaryCount{counters[0].get()};
        // increase counter
        primaryCount->TryComeToPower(0);
        sharpen::AwaitableFuture<sharpen::ConsensusResult> future;
        primary->Advance();
        primary->WaitNextConsensus(future);
        sharpen::Delay(std::chrono::seconds{1});
        bool writable{primary->Writable()};
        if (writable) {
            // close all hosts
            for (auto begin = rafts.begin(), end = rafts.end(); begin != end; ++begin) {
                auto raft{begin->get()};
                raft->ReleasePeers();
            }
            for (auto begin = hosts.begin(), end = hosts.end(); begin != end; ++begin) {
                auto host{begin->get()};
                host->Stop();
            }
            for (auto begin = process.begin(), end = process.end(); begin != end; ++begin) {
                auto future{begin->get()};
                future->WaitAsync();
            }
            // remove files
            for (std::uint16_t i = beginPort; i != endPort; ++i) {
                RemoveLogStorage(i);
                RemoveStatusMap(i);
            }
            return this->Fail("should not be writable");
        }
        primaryCount->Abdicate();
        primary->Advance();
        future.Await();
        writable = primary->Writable();
        // close all hosts
        for (auto begin = rafts.begin(), end = rafts.end(); begin != end; ++begin) {
            auto raft{begin->get()};
            raft->ReleasePeers();
        }
        for (auto begin = hosts.begin(), end = hosts.end(); begin != end; ++begin) {
            auto host{begin->get()};
            host->Stop();
        }
        for (auto begin = process.begin(), end = process.end(); begin != end; ++begin) {
            auto future{begin->get()};
            future->WaitAsync();
        }
        // remove files
        for (std::uint16_t i = beginPort; i != endPort; ++i) {
            RemoveLogStorage(i);
            RemoveStatusMap(i);
        }
        return this->Assert(writable, "should be writable");
    }
};

int Entry() {
    sharpen::StartupNetSupport();
    simpletest::TestRunner runner{simpletest::DisplayMode::Blocked};
    runner.Register<BasicElectionTest>();
    runner.Register<FaultElectionTest>();
    runner.Register<SplitBrainTest>();
    runner.Register<PrevoteElectionTest>();
    runner.Register<FaultPrevoteTest>();
    runner.Register<BasicLeaderBalanceTest>();
    runner.Register<FaultBalanceTest>();
    int code{runner.Run()};
    sharpen::CleanupNetSupport();
    return code;
}

int main() {
    sharpen::EventEngine &engine{sharpen::EventEngine::SetupEngine()};
    return engine.StartupWithCode(&Entry);
}