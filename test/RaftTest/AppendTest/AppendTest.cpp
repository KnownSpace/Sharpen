#include <common/LogStep.hpp>
#include <common/RaftStep.hpp>
#include <common/RaftTool.hpp>
#include <sharpen/AsyncOps.hpp>
#include <sharpen/BinarySerializable.hpp>
#include <sharpen/BufferWriter.hpp>
#include <sharpen/DebugTools.hpp>
#include <sharpen/EventEngine.hpp>
#include <sharpen/GenericMailParserFactory.hpp>
#include <sharpen/IConsensus.hpp>
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
#include <chrono>
#include <cinttypes>
#include <memory>
#include <sstream>
#include <vector>


static const std::uint32_t magicNumber{0x2333};

static const std::uint16_t beginPort{10801};

static const std::uint16_t endPort{10803};

static constexpr std::size_t appendTestCount{60};

static constexpr std::size_t batchSize{20};

static constexpr std::size_t pipelineLength{2};

static constexpr std::size_t benchmarkCount{1 * 1000};

static constexpr std::size_t benchmarkTime{10};

static constexpr std::size_t benchmarkClientCount{10};

static constexpr std::size_t benchmarkEntrySize{32 * 1024};

static std::shared_ptr<sharpen::IConsensus> CreateRaft(std::uint16_t port) {
    sharpen::RaftOption raftOpt;
    raftOpt.SetBatchSize(batchSize);
    raftOpt.SetLearner(false);
    raftOpt.SetPrevote(false);
    auto raft{CreateRaft(port, magicNumber, nullptr, nullptr, raftOpt, false)};
    raft->ConfiguratePeers(
        &ConfigPeers, port, beginPort, endPort, &raft->GetReceiver(), magicNumber, false);
    return raft;
}

static std::shared_ptr<sharpen::IConsensus> CreatePipelineRaft(std::uint16_t port) {
    sharpen::RaftOption raftOpt;
    raftOpt.SetBatchSize(batchSize);
    raftOpt.SetLearner(false);
    raftOpt.SetPrevote(false);
    raftOpt.SetPipelineLength(pipelineLength);
    auto raft{CreateRaft(port, magicNumber, nullptr, nullptr, raftOpt, true)};
    raft->ConfiguratePeers(
        &ConfigPeers, port, beginPort, endPort, &raft->GetReceiver(), magicNumber, true);
    return raft;
}

static std::shared_ptr<sharpen::IConsensus> CreateLeaseRaft(std::uint16_t port) {
    sharpen::RaftOption raftOpt;
    raftOpt.SetBatchSize(batchSize);
    raftOpt.SetLearner(false);
    raftOpt.SetPrevote(false);
    raftOpt.EnableLeaseAwareness();
    auto raft{CreateRaft(port, magicNumber, nullptr, nullptr, raftOpt, false)};
    raft->ConfiguratePeers(
        &ConfigPeers, port, beginPort, endPort, &raft->GetReceiver(), magicNumber, false);
    return raft;
}

static std::shared_ptr<sharpen::IConsensus> CreateLeasePipelineRaft(std::uint16_t port) {
    sharpen::RaftOption raftOpt;
    raftOpt.SetBatchSize(batchSize);
    raftOpt.SetLearner(false);
    raftOpt.SetPrevote(false);
    raftOpt.SetPipelineLength(pipelineLength);
    raftOpt.EnableLeaseAwareness();
    auto raft{CreateRaft(port, magicNumber, nullptr, nullptr, raftOpt, true)};
    raft->ConfiguratePeers(
        &ConfigPeers, port, beginPort, endPort, &raft->GetReceiver(), magicNumber, false);
    return raft;
}

static std::unique_ptr<sharpen::IHostPipeline> ConfigPipeline(
    std::shared_ptr<sharpen::IConsensus> raft) {
    std::unique_ptr<sharpen::IHostPipeline> pipe{new (std::nothrow) sharpen::SimpleHostPipeline{}};
    pipe->Register<LogStep>();
    pipe->Register<RaftStep>(magicNumber, std::move(raft));
    return pipe;
}


static std::unique_ptr<sharpen::IHostPipeline> ConfigNotLoggingPipeline(
    std::shared_ptr<sharpen::IConsensus> raft) {
    std::unique_ptr<sharpen::IHostPipeline> pipe{new (std::nothrow) sharpen::SimpleHostPipeline{}};
    std::unique_ptr<RaftStep> step{new (std::nothrow) RaftStep{magicNumber, std::move(raft)}};
    step->DisableLogging();
    pipe->Register(std::move(step));
    return pipe;
}

static std::unique_ptr<sharpen::TcpHost> CreateHost(std::uint16_t port,
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

static std::unique_ptr<sharpen::TcpHost> CreateNotLoggingHost(
    std::uint16_t port, std::shared_ptr<sharpen::IConsensus> raft) {
    sharpen::IpEndPoint endPoint;
    endPoint.SetAddrByString("127.0.0.1");
    endPoint.SetPort(port);
    sharpen::IpTcpStreamFactory streamFactory{endPoint};
    std::unique_ptr<sharpen::TcpHost> host{new (std::nothrow) sharpen::TcpHost{streamFactory}};
    if (!host) {
        throw std::bad_alloc{};
    }
    host->ConfiguratePipeline(&ConfigNotLoggingPipeline, std::move(raft));
    return host;
}

static void PrintDebugInfo() {
    sharpen::SyncPrintf("Batch size: %zu\nPieline Length: %zu\n",batchSize,pipelineLength);
}

class BasicHeartbeatTest : public simpletest::ITypenamedTest<BasicHeartbeatTest> {
private:
    using Self = BasicHeartbeatTest;

public:
    BasicHeartbeatTest() noexcept = default;

    ~BasicHeartbeatTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        PrintDebugInfo();
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
        std::size_t count{0};
        for (std::size_t i = 0; i != appendTestCount && writable; ++i) {
            sharpen::SyncPrintf("Heartbeat %zu\n", i);
            primary->Advance();
            for (auto begin = rafts.begin() + 1, end = rafts.end(); begin != end; ++begin) {
                auto backup{begin->get()};
                backup->WaitNextConsensus();
            }
            count += 1;
            writable = primary->Writable();
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
        // remove files
        for (std::uint16_t i = beginPort; i != endPort + 1; ++i) {
            RemoveLogStorage(i);
            RemoveStatusMap(i);
        }
        return this->Assert(count == appendTestCount, "count should equal with appendTestCount");
    }
};

class FaultHeartbeatTest : public simpletest::ITypenamedTest<FaultHeartbeatTest> {
private:
    using Self = FaultHeartbeatTest;

public:
    FaultHeartbeatTest() noexcept = default;

    ~FaultHeartbeatTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        PrintDebugInfo();
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
        std::size_t count{0};
        for (std::size_t i = 0; i != appendTestCount && writable; ++i) {
            sharpen::SyncPrintf("Heartbeat %zu\n", i);
            primary->Advance();
            for (auto begin = rafts.begin() + 1, end = rafts.end(); begin != end; ++begin) {
                auto backup{begin->get()};
                backup->WaitNextConsensus();
            }
            count += 1;
            writable = primary->Writable();
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
        // remove files
        for (std::uint16_t i = beginPort; i != endPort; ++i) {
            RemoveLogStorage(i);
            RemoveStatusMap(i);
        }
        return this->Assert(count == appendTestCount, "count should equal with appendTestCount");
    }
};

class BasicAppendTest : public simpletest::ITypenamedTest<BasicAppendTest> {
private:
    using Self = BasicAppendTest;

public:
    BasicAppendTest() noexcept = default;

    ~BasicAppendTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        PrintDebugInfo();
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
        std::size_t count{0};
        for (std::size_t i = 0; i != appendTestCount && writable; ++i) {
            sharpen::SyncPrintf("AppendEntires %zu\n", i);
            sharpen::LogBatch batch;
            sharpen::ByteBuffer log;
            log.Printf("Index:%zu", i);
            batch.Append(std::move(log));
            primary->Write(batch);
            primary->Advance();
            for (auto begin = rafts.begin() + 1, end = rafts.end(); begin != end; ++begin) {
                auto backup{begin->get()};
                backup->WaitNextConsensus();
            }
            count += 1;
            writable = primary->Writable();
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
        // remove files
        for (std::uint16_t i = beginPort; i != endPort + 1; ++i) {
            RemoveLogStorage(i);
            RemoveStatusMap(i);
        }
        return this->Assert(count == appendTestCount, "count should equal with appendTestCount");
    }
};

class FaultAppendTest : public simpletest::ITypenamedTest<FaultAppendTest> {
private:
    using Self = FaultAppendTest;

public:
    FaultAppendTest() noexcept = default;

    ~FaultAppendTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        PrintDebugInfo();
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
        std::size_t count{0};
        for (std::size_t i = 0; i != appendTestCount && writable; ++i) {
            sharpen::SyncPrintf("AppendEntires %zu\n", i);
            sharpen::LogBatch batch;
            sharpen::ByteBuffer log;
            log.Printf("Index:%zu", i);
            batch.Append(std::move(log));
            primary->Write(batch);
            primary->Advance();
            for (auto begin = rafts.begin() + 1, end = rafts.end(); begin != end; ++begin) {
                auto backup{begin->get()};
                backup->WaitNextConsensus();
            }
            count += 1;
            writable = primary->Writable();
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
        // remove files
        for (std::uint16_t i = beginPort; i != endPort; ++i) {
            RemoveLogStorage(i);
            RemoveStatusMap(i);
        }
        return this->Assert(count == appendTestCount, "count should equal with appendTestCount");
    }
};

class RecoveryAppendTest : public simpletest::ITypenamedTest<RecoveryAppendTest> {
private:
    using Self = RecoveryAppendTest;

public:
    RecoveryAppendTest() noexcept = default;

    ~RecoveryAppendTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        PrintDebugInfo();
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
        sharpen::IHost *faultHost{hosts.rbegin()->get()};
        sharpen::IConsensus *faultRaft{rafts.rbegin()->get()};
        for (auto begin = hosts.begin(), end = hosts.end(); begin != end; ++begin) {
            sharpen::IHost *host{begin->get()};
            sharpen::SyncPrintf("Host:%p Fault:%p\n", host, faultHost);
            if (host != faultHost) {
                auto future{sharpen::Async([host]() { host->Run(); })};
                process.emplace_back(std::move(future));
            }
        }
        auto primary{rafts[0].get()};
        primary->Advance();
        primary->WaitNextConsensus();
        bool writable{primary->Writable()};
        std::size_t count{0};
        for (std::size_t i = 0; i != appendTestCount && writable; ++i) {
            sharpen::SyncPrintf("AppendEntires %zu\n", i);
            sharpen::LogBatch batch;
            sharpen::ByteBuffer log{};
            log.Printf("Index:%zu", i);
            batch.Append(std::move(log));
            primary->Write(batch);
            primary->Advance();
            primary->WaitNextConsensus();
            count += 1;
            writable = primary->Writable();
        }
        {
            auto future{sharpen::Async([faultHost]() {
                sharpen::SyncPuts("Run fault host");
                faultHost->Run();
            })};
            process.emplace_back(std::move(future));
        }
        sharpen::SyncPuts("Begin recovery");
        while (faultRaft->ImmutableLogs().GetLastIndex() !=
               primary->ImmutableLogs().GetLastIndex()) {
            primary->Advance();
            faultRaft->WaitNextConsensus();
            writable = primary->Writable();
            sharpen::SyncPrintf("Recovery to %zu/%zu\n",
                                faultRaft->ImmutableLogs().GetLastIndex(),
                                primary->ImmutableLogs().GetLastIndex());
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
        // remove files
        for (std::uint16_t i = beginPort; i != endPort + 1; ++i) {
            RemoveLogStorage(i);
            RemoveStatusMap(i);
        }
        return this->Assert(count == appendTestCount, "count should equal with appendTestCount");
    }
};

class PipelineAppendTest : public simpletest::ITypenamedTest<PipelineAppendTest> {
private:
    using Self = PipelineAppendTest;

public:
    PipelineAppendTest() noexcept = default;

    ~PipelineAppendTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        PrintDebugInfo();
        std::vector<std::shared_ptr<sharpen::IConsensus>> rafts;
        rafts.reserve(3);
        std::vector<std::unique_ptr<sharpen::IHost>> hosts;
        hosts.reserve(3);
        std::vector<sharpen::AwaitableFuturePtr<void>> process;
        process.reserve(3);
        for (std::uint16_t i = beginPort; i != endPort + 1; ++i) {
            auto raft{CreatePipelineRaft(i)};
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
        std::size_t count{0};
        for (std::size_t i = 0; i != appendTestCount && writable; ++i) {
            sharpen::SyncPrintf("AppendEntires %zu\n", i);
            sharpen::LogBatch batch;
            sharpen::ByteBuffer log;
            log.Printf("Index:%zu", i);
            batch.Append(std::move(log));
            primary->Write(batch);
            primary->Advance();
            for (auto begin = rafts.begin() + 1, end = rafts.end(); begin != end; ++begin) {
                auto backup{begin->get()};
                backup->WaitNextConsensus();
            }
            count += 1;
            writable = primary->Writable();
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
        // remove files
        for (std::uint16_t i = beginPort; i != endPort + 1; ++i) {
            RemoveLogStorage(i);
            RemoveStatusMap(i);
        }
        return this->Assert(count == appendTestCount, "count should equal with appendTestCount");
    }
};

class FaultPipelineAppendTest : public simpletest::ITypenamedTest<FaultPipelineAppendTest> {
private:
    using Self = FaultPipelineAppendTest;

public:
    FaultPipelineAppendTest() noexcept = default;

    ~FaultPipelineAppendTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        PrintDebugInfo();
        std::vector<std::shared_ptr<sharpen::IConsensus>> rafts;
        rafts.reserve(2);
        std::vector<std::unique_ptr<sharpen::IHost>> hosts;
        hosts.reserve(2);
        std::vector<sharpen::AwaitableFuturePtr<void>> process;
        process.reserve(2);
        for (std::uint16_t i = beginPort; i != endPort; ++i) {
            auto raft{CreatePipelineRaft(i)};
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
        std::size_t count{0};
        for (std::size_t i = 0; i != appendTestCount && writable; ++i) {
            sharpen::SyncPrintf("AppendEntires %zu\n", i);
            sharpen::LogBatch batch;
            sharpen::ByteBuffer log;
            log.Printf("Index:%zu", i);
            batch.Append(std::move(log));
            primary->Write(batch);
            primary->Advance();
            for (auto begin = rafts.begin() + 1, end = rafts.end(); begin != end; ++begin) {
                auto backup{begin->get()};
                backup->WaitNextConsensus();
            }
            count += 1;
            writable = primary->Writable();
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
        // remove files
        for (std::uint16_t i = beginPort; i != endPort; ++i) {
            RemoveLogStorage(i);
            RemoveStatusMap(i);
        }
        return this->Assert(count == appendTestCount, "count should equal with appendTestCount");
    }
};

class RecoveryPipelineAppendTest : public simpletest::ITypenamedTest<RecoveryPipelineAppendTest> {
private:
    using Self = RecoveryPipelineAppendTest;

public:
    RecoveryPipelineAppendTest() noexcept = default;

    ~RecoveryPipelineAppendTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        PrintDebugInfo();
        std::vector<std::shared_ptr<sharpen::IConsensus>> rafts;
        rafts.reserve(3);
        std::vector<std::unique_ptr<sharpen::IHost>> hosts;
        hosts.reserve(3);
        std::vector<sharpen::AwaitableFuturePtr<void>> process;
        process.reserve(3);
        for (std::uint16_t i = beginPort; i != endPort + 1; ++i) {
            auto raft{CreatePipelineRaft(i)};
            rafts.emplace_back(raft);
            auto host{CreateHost(i, raft)};
            hosts.emplace_back(std::move(host));
        }
        sharpen::IHost *faultHost{hosts.rbegin()->get()};
        sharpen::IConsensus *faultRaft{rafts.rbegin()->get()};
        for (auto begin = hosts.begin(), end = hosts.end(); begin != end; ++begin) {
            sharpen::IHost *host{begin->get()};
            sharpen::SyncPrintf("Host:%p Fault:%p\n", host, faultHost);
            if (host != faultHost) {
                auto future{sharpen::Async([host]() { host->Run(); })};
                process.emplace_back(std::move(future));
            }
        }
        auto primary{rafts[0].get()};
        primary->Advance();
        primary->WaitNextConsensus();
        bool writable{primary->Writable()};
        std::size_t count{0};
        for (std::size_t i = 0; i != appendTestCount && writable; ++i) {
            sharpen::SyncPrintf("AppendEntires %zu\n", i);
            sharpen::LogBatch batch;
            sharpen::ByteBuffer log;
            log.Printf("Index:%zu", i);
            batch.Append(std::move(log));
            primary->Write(batch);
            primary->Advance();
            primary->WaitNextConsensus();
            count += 1;
            writable = primary->Writable();
        }
        {
            auto future{sharpen::Async([faultHost]() {
                sharpen::SyncPuts("Run fault host");
                faultHost->Run();
            })};
            process.emplace_back(std::move(future));
        }
        sharpen::SyncPuts("Begin recovery");
        while (faultRaft->ImmutableLogs().GetLastIndex() !=
               primary->ImmutableLogs().GetLastIndex()) {
            primary->Advance();
            faultRaft->WaitNextConsensus();
            writable = primary->Writable();
            sharpen::SyncPrintf("Recovery to %zu/%zu\n",
                                faultRaft->ImmutableLogs().GetLastIndex(),
                                primary->ImmutableLogs().GetLastIndex());
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
        // remove files
        for (std::uint16_t i = beginPort; i != endPort + 1; ++i) {
            RemoveLogStorage(i);
            RemoveStatusMap(i);
        }
        return this->Assert(count == appendTestCount, "count should equal with appendTestCount");
    }
};

class BasicLeaseTest : public simpletest::ITypenamedTest<BasicLeaseTest> {
private:
    using Self = BasicLeaseTest;

public:
    BasicLeaseTest() noexcept = default;

    ~BasicLeaseTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        PrintDebugInfo();
        std::vector<std::shared_ptr<sharpen::IConsensus>> rafts;
        rafts.reserve(3);
        std::vector<std::unique_ptr<sharpen::IHost>> hosts;
        hosts.reserve(3);
        std::vector<sharpen::AwaitableFuturePtr<void>> process;
        process.reserve(3);
        for (std::uint16_t i = beginPort; i != endPort + 1; ++i) {
            auto raft{CreateLeaseRaft(i)};
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
        std::size_t count{0};
        for (std::size_t i = 0; i != appendTestCount && writable; ++i) {
            sharpen::SyncPrintf("Lease %zu\n", i);
            primary->Advance();
            auto result{primary->WaitNextConsensus()};
            if (!result.IsLeaseConfirmed()) {
                break;
            }
            count += 1;
            writable = primary->Writable();
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
        // remove files
        for (std::uint16_t i = beginPort; i != endPort + 1; ++i) {
            RemoveLogStorage(i);
            RemoveStatusMap(i);
        }
        return this->Assert(count == appendTestCount, "count should equal with appendTestCount");
    }
};

class FaultLeaseTest : public simpletest::ITypenamedTest<FaultLeaseTest> {
private:
    using Self = FaultLeaseTest;

public:
    FaultLeaseTest() noexcept = default;

    ~FaultLeaseTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        PrintDebugInfo();
        std::vector<std::shared_ptr<sharpen::IConsensus>> rafts;
        rafts.reserve(2);
        std::vector<std::unique_ptr<sharpen::IHost>> hosts;
        hosts.reserve(2);
        std::vector<sharpen::AwaitableFuturePtr<void>> process;
        process.reserve(2);
        for (std::uint16_t i = beginPort; i != endPort; ++i) {
            auto raft{CreateLeaseRaft(i)};
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
        std::size_t count{0};
        for (std::size_t i = 0; i != appendTestCount && writable; ++i) {
            sharpen::SyncPrintf("Lease %zu\n", i);
            primary->Advance();
            auto result{primary->WaitNextConsensus()};
            if (!result.IsLeaseConfirmed()) {
                break;
            }
            count += 1;
            writable = primary->Writable();
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
        // remove files
        for (std::uint16_t i = beginPort; i != endPort; ++i) {
            RemoveLogStorage(i);
            RemoveStatusMap(i);
        }
        return this->Assert(count == appendTestCount, "count should equal with appendTestCount");
    }
};

class RttBenchmark : public simpletest::ITypenamedTest<RttBenchmark> {
private:
    using Self = RttBenchmark;

public:
    RttBenchmark() noexcept = default;

    ~RttBenchmark() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        PrintDebugInfo();
        std::vector<std::shared_ptr<sharpen::IConsensus>> rafts;
        rafts.reserve(3);
        std::vector<std::unique_ptr<sharpen::IHost>> hosts;
        hosts.reserve(3);
        std::vector<sharpen::AwaitableFuturePtr<void>> process;
        process.reserve(3);
        for (std::uint16_t i = beginPort; i != endPort + 1; ++i) {
            auto raft{CreateLeaseRaft(i)};
            rafts.emplace_back(raft);
            auto host{CreateNotLoggingHost(i, raft)};
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
        auto firstTp{std::chrono::system_clock::now()};
        for (std::size_t i = 0; i != benchmarkCount; ++i) {
            primary->Advance();
            primary->WaitNextConsensus();
            writable = primary->Writable();
        }
        auto secondTp{std::chrono::system_clock::now()};
        std::int64_t count{
            std::chrono::duration_cast<std::chrono::milliseconds>(secondTp - firstTp).count()};
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
        sharpen::SyncPrintf("Using %" PRId64 " ms to execute %zu rounds\n", count, benchmarkCount);
        return this->Success();
    }
};

class PipelineRttBenchmark : public simpletest::ITypenamedTest<PipelineRttBenchmark> {
private:
    using Self = PipelineRttBenchmark;

public:
    PipelineRttBenchmark() noexcept = default;

    ~PipelineRttBenchmark() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        PrintDebugInfo();
        std::vector<std::shared_ptr<sharpen::IConsensus>> rafts;
        rafts.reserve(3);
        std::vector<std::unique_ptr<sharpen::IHost>> hosts;
        hosts.reserve(3);
        std::vector<sharpen::AwaitableFuturePtr<void>> process;
        process.reserve(3);
        for (std::uint16_t i = beginPort; i != endPort + 1; ++i) {
            auto raft{CreateLeasePipelineRaft(i)};
            rafts.emplace_back(raft);
            auto host{CreateNotLoggingHost(i, raft)};
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
        auto firstTp{std::chrono::system_clock::now()};
        for (std::size_t i = 0; i != benchmarkCount; ++i) {
            primary->Advance();
            if ((i + 1) % pipelineLength == 0) {
                primary->WaitNextConsensus();
            }
            writable = primary->Writable();
        }
        auto secondTp{std::chrono::system_clock::now()};
        std::int64_t count{
            std::chrono::duration_cast<std::chrono::milliseconds>(secondTp - firstTp).count()};
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
        sharpen::SyncPrintf("Using %" PRId64 " ms to execute %zu rounds\n", count, benchmarkCount);
        return this->Success();
    }
};

class BasicAppendBenchmark : public simpletest::ITypenamedTest<BasicAppendBenchmark> {
private:
    using Self = BasicAppendBenchmark;

public:
    BasicAppendBenchmark() noexcept = default;

    ~BasicAppendBenchmark() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        PrintDebugInfo();
        std::vector<std::shared_ptr<sharpen::IConsensus>> rafts;
        rafts.reserve(3);
        std::vector<std::unique_ptr<sharpen::IHost>> hosts;
        hosts.reserve(3);
        std::vector<sharpen::AwaitableFuturePtr<void>> process;
        process.reserve(3);
        for (std::uint16_t i = beginPort; i != endPort + 1; ++i) {
            auto raft{CreateLeaseRaft(i)};
            rafts.emplace_back(raft);
            auto host{CreateNotLoggingHost(i, raft)};
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
        sharpen::Future<bool> timerFuture;
        sharpen::TimerPtr timer{sharpen::MakeTimer()};
        timer->WaitAsync(timerFuture, std::chrono::seconds{benchmarkTime});
        std::vector<sharpen::AwaitableFuturePtr<void>> clients;
        clients.reserve(benchmarkClientCount);
        std::size_t rounds{0};
        for (std::size_t i = 0; i != benchmarkClientCount; ++i) {
            clients.emplace_back(sharpen::Async([&timerFuture, primary]() {
                while (timerFuture.IsPending()) {
                    sharpen::LogBatch batch;
                    sharpen::ByteBuffer log{benchmarkEntrySize};
                    batch.Append(std::move(log));
                    primary->Write(batch);
                }
            }));
        }
        auto advancer{sharpen::Async([&timerFuture, primary, &rounds]() {
            while (timerFuture.IsPending()) {
                primary->Advance();
                primary->WaitNextConsensus();
                rounds += 1;
            }
        })};
        for (auto begin = clients.begin(), end = clients.end(); begin != end; ++begin) {
            begin->get()->Await();
        }
        advancer->Await();
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
        constexpr static std::size_t MB{1024 * 1024};
        constexpr static std::size_t KB{1024};
        std::printf("Commit %zu entires with %zu clients in %zu second and %zu rounds, total size "
                    "is %zu MB, entry size %zu KB\n",
                    primary->GetCommitIndex(),
                    benchmarkClientCount,
                    benchmarkTime,
                    rounds,
                    benchmarkEntrySize * primary->GetCommitIndex() / MB,benchmarkEntrySize/KB);
        return this->Success();
    }
};

class PipelineAppendBenchmark : public simpletest::ITypenamedTest<PipelineAppendBenchmark> {
private:
    using Self = PipelineAppendBenchmark;

public:
    PipelineAppendBenchmark() noexcept = default;

    ~PipelineAppendBenchmark() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        PrintDebugInfo();
        std::vector<std::shared_ptr<sharpen::IConsensus>> rafts;
        rafts.reserve(3);
        std::vector<std::unique_ptr<sharpen::IHost>> hosts;
        hosts.reserve(3);
        std::vector<sharpen::AwaitableFuturePtr<void>> process;
        process.reserve(3);
        for (std::uint16_t i = beginPort; i != endPort + 1; ++i) {
            auto raft{CreateLeasePipelineRaft(i)};
            rafts.emplace_back(raft);
            auto host{CreateNotLoggingHost(i, raft)};
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
        sharpen::Future<bool> timerFuture;
        sharpen::TimerPtr timer{sharpen::MakeTimer()};
        timer->WaitAsync(timerFuture, std::chrono::seconds{benchmarkTime});
        std::vector<sharpen::AwaitableFuturePtr<void>> clients;
        clients.reserve(benchmarkClientCount);
        std::size_t rounds{0};
        for (std::size_t i = 0; i != benchmarkClientCount; ++i) {
            clients.emplace_back(sharpen::Async([&timerFuture, primary]() {
                while (timerFuture.IsPending()) {
                    sharpen::LogBatch batch;
                    sharpen::ByteBuffer log{benchmarkEntrySize};
                    batch.Append(std::move(log));
                    primary->Write(batch);
                }
            }));
        }
        auto advancer{sharpen::Async([&timerFuture, primary, &rounds]() {
            while (timerFuture.IsPending()) {
                primary->Advance();
                rounds += 1;
                primary->WaitNextConsensus();
            }
        })};
        for (auto begin = clients.begin(), end = clients.end(); begin != end; ++begin) {
            begin->get()->Await();
        }
        advancer->Await();
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
        constexpr static std::size_t MB{1024 * 1024};
        constexpr static std::size_t KB{1024};
        std::printf("Commit %zu entires with %zu clients in %zu second and %zu rounds, total size "
                    "is %zu MB, entry size %zu KB\n",
                    primary->GetCommitIndex(),
                    benchmarkClientCount,
                    benchmarkTime,
                    rounds,
                    benchmarkEntrySize * primary->GetCommitIndex() / MB,benchmarkEntrySize/KB);
        return this->Success();
    }
};

int Entry() {
    sharpen::StartupNetSupport();
    simpletest::TestRunner runner{simpletest::DisplayMode::Blocked};
    runner.Register<BasicHeartbeatTest>();
    runner.Register<FaultHeartbeatTest>();
    runner.Register<BasicAppendTest>();
    runner.Register<FaultAppendTest>();
    runner.Register<RecoveryAppendTest>();
    runner.Register<PipelineAppendTest>();
    runner.Register<FaultPipelineAppendTest>();
    runner.Register<RecoveryPipelineAppendTest>();
    runner.Register<BasicLeaseTest>();
    runner.Register<FaultLeaseTest>();
    runner.Register<RttBenchmark>();
    runner.Register<PipelineRttBenchmark>();
    runner.Register<BasicAppendBenchmark>();
    runner.Register<PipelineAppendBenchmark>();
    int code{runner.Run()};
    sharpen::CleanupNetSupport();
    return code;
}

int main() {
    sharpen::EventEngine &engine{sharpen::EventEngine::SetupEngine()};
    return engine.StartupWithCode(&Entry);
}