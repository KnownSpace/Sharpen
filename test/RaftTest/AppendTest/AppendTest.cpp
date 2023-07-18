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

static constexpr std::size_t appendTestCount{60};

static constexpr std::size_t batchSize{20};

static constexpr std::size_t pipelineLength{2};

static std::shared_ptr<sharpen::IConsensus> CreateRaft(std::uint16_t port) {
    sharpen::RaftOption raftOpt;
    raftOpt.SetBatchSize(batchSize);
    raftOpt.SetLearner(false);
    raftOpt.SetPrevote(false);
    auto raft{CreateRaft(port, magicNumber, nullptr,nullptr, raftOpt,false)};
    raft->ConfiguratePeers(
        &ConfigPeers, port, beginPort, endPort, &raft->GetReceiver(), magicNumber,false);
    return raft;
}

static std::shared_ptr<sharpen::IConsensus> CreatePipelineRaft(std::uint16_t port) {
    sharpen::RaftOption raftOpt;
    raftOpt.SetBatchSize(batchSize);
    raftOpt.SetLearner(false);
    raftOpt.SetPrevote(false);
    raftOpt.SetPipelineLength(pipelineLength);
    auto raft{CreateRaft(port, magicNumber, nullptr,nullptr, raftOpt,true)};
    raft->ConfiguratePeers(
        &ConfigPeers, port, beginPort, endPort, &raft->GetReceiver(), magicNumber,true);
    return raft;
}

static std::shared_ptr<sharpen::IConsensus> CreateLeaseRaft(std::uint16_t port) {
    sharpen::RaftOption raftOpt;
    raftOpt.SetBatchSize(batchSize);
    raftOpt.SetLearner(false);
    raftOpt.SetPrevote(false);
    raftOpt.EnableLeaseAwareness();
    auto raft{CreateRaft(port, magicNumber,nullptr, nullptr, raftOpt,false)};
    raft->ConfiguratePeers(
        &ConfigPeers, port, beginPort, endPort, &raft->GetReceiver(), magicNumber,false);
    return raft;
}

static std::unique_ptr<sharpen::IHostPipeline> ConfigPipeline(std::shared_ptr<sharpen::IConsensus> raft) {
    std::unique_ptr<sharpen::IHostPipeline> pipe{new (std::nothrow) sharpen::SimpleHostPipeline{}};
    pipe->Register<LogStep>();
    pipe->Register<RaftStep>(magicNumber, std::move(raft));
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

static void WaitForPorts() {
    sharpen::Delay(std::chrono::seconds{3});
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
        WaitForPorts();
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
        WaitForPorts();
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
        WaitForPorts();
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
        WaitForPorts();
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
        WaitForPorts();
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
            sharpen::SyncPrintf("Recovery to %zu/%zu\n", faultRaft->ImmutableLogs().GetLastIndex(),primary->ImmutableLogs().GetLastIndex());
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

class PipelineAppendTest:public simpletest::ITypenamedTest<PipelineAppendTest>
{
private:
    using Self = PipelineAppendTest;

public:

    PipelineAppendTest() noexcept = default;

    ~PipelineAppendTest() noexcept = default;

    inline const Self &Const() const noexcept
    {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept
    {
        WaitForPorts();
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

class FaultPipelineAppendTest:public simpletest::ITypenamedTest<FaultPipelineAppendTest>
{
private:
    using Self = FaultPipelineAppendTest;

public:

    FaultPipelineAppendTest() noexcept = default;

    ~FaultPipelineAppendTest() noexcept = default;

    inline const Self &Const() const noexcept
    {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept
    {
        WaitForPorts();
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

class RecoveryPipelineAppendTest:public simpletest::ITypenamedTest<RecoveryPipelineAppendTest>
{
private:
    using Self = RecoveryPipelineAppendTest;

public:

    RecoveryPipelineAppendTest() noexcept = default;

    ~RecoveryPipelineAppendTest() noexcept = default;

    inline const Self &Const() const noexcept
    {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept
    {
        WaitForPorts();
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
            sharpen::SyncPrintf("Recovery to %zu/%zu\n", faultRaft->ImmutableLogs().GetLastIndex(),primary->ImmutableLogs().GetLastIndex());
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

class BasicLeaseTest:public simpletest::ITypenamedTest<BasicLeaseTest>
{
private:
    using Self = BasicLeaseTest;

public:

    BasicLeaseTest() noexcept = default;

    ~BasicLeaseTest() noexcept = default;

    inline const Self &Const() const noexcept
    {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept
    {
        WaitForPorts();
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

class FaultLeaseTest:public simpletest::ITypenamedTest<FaultLeaseTest>
{
private:
    using Self = FaultLeaseTest;

public:

    FaultLeaseTest() noexcept = default;

    ~FaultLeaseTest() noexcept = default;

    inline const Self &Const() const noexcept
    {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept
    {
        WaitForPorts();
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
    int code{runner.Run()};
    sharpen::CleanupNetSupport();
    return code;
}

int main() {
    sharpen::EventEngine &engine{sharpen::EventEngine::SetupEngine()};
    return engine.StartupWithCode(&Entry);
}