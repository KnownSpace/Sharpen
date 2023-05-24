#include <common/BatchHandler.hpp>
#include <common/DropStep.hpp>
#include <common/EchoReceiver.hpp>
#include <common/HandleStep.hpp>
#include <common/LogStep.hpp>
#include <sharpen/BinarySerializable.hpp>
#include <sharpen/BufferWriter.hpp>
#include <sharpen/DebugTools.hpp>
#include <sharpen/EventEngine.hpp>
#include <sharpen/GenericMailParserFactory.hpp>
#include <sharpen/IMailReceiver.hpp>
#include <sharpen/IpTcpActorBuilder.hpp>
#include <sharpen/IpTcpStreamFactory.hpp>
#include <sharpen/SimpleHostPipeline.hpp>
#include <sharpen/SingleWorkerGroup.hpp>
#include <sharpen/TcpActor.hpp>
#include <sharpen/TcpHost.hpp>
#include <sharpen/TimerOps.hpp>
#include <simpletest/TestRunner.hpp>
#include <vector>

static const std::uint32_t magicNumber{0x2333};

std::unique_ptr<sharpen::IHostPipeline> ConfigPipeline(
    std::function<void(sharpen::INetStreamChannel *, sharpen::Mail)> handler) {
    std::unique_ptr<sharpen::IHostPipeline> pipe{new (std::nothrow) sharpen::SimpleHostPipeline{}};
    pipe->Register<LogStep>();
    pipe->Register<HandleStep>(magicNumber, std::move(handler));
    return pipe;
}

std::unique_ptr<sharpen::IHostPipeline> ConfigDrop() {
    std::unique_ptr<sharpen::IHostPipeline> pipe{new (std::nothrow) sharpen::SimpleHostPipeline{}};
    pipe->Register<LogStep>();
    pipe->Register<DropStep>();
    return pipe;
}

std::unique_ptr<sharpen::TcpHost> CreateHost(
    std::uint16_t port, std::function<void(sharpen::INetStreamChannel *, sharpen::Mail)> handler) {
    sharpen::IpEndPoint endPoint;
    endPoint.SetAddrByString("127.0.0.1");
    endPoint.SetPort(port);
    sharpen::IpTcpStreamFactory streamFactory{endPoint};
    std::unique_ptr<sharpen::TcpHost> host{new (std::nothrow) sharpen::TcpHost{streamFactory}};
    if (!host) {
        throw std::bad_alloc{};
    }
    host->ConfiguratePipeline(&ConfigPipeline, std::move(handler));
    return host;
}

std::unique_ptr<sharpen::TcpHost> CreateDropHost(
    std::uint16_t port) {
    sharpen::IpEndPoint endPoint;
    endPoint.SetAddrByString("127.0.0.1");
    endPoint.SetPort(port);
    sharpen::IpTcpStreamFactory streamFactory{endPoint};
    std::unique_ptr<sharpen::TcpHost> host{new (std::nothrow) sharpen::TcpHost{streamFactory}};
    if (!host) {
        throw std::bad_alloc{};
    }
    host->ConfiguratePipeline(&ConfigDrop);
    return host;
}

std::unique_ptr<sharpen::IRemoteActor> CreateActor(std::uint16_t port,
                                                   bool pipeline,
                                                   sharpen::IMailReceiver &receiver) {
    sharpen::IpTcpActorBuilder builder{pipeline};
    sharpen::IpEndPoint remote;
    remote.SetAddrByString("127.0.0.1");
    remote.SetPort(port);
    builder.PrepareRemote(remote);
    std::shared_ptr<sharpen::IMailParserFactory> factory{
        std::make_shared<sharpen::GenericMailParserFactory>(magicNumber)};
    builder.PrepareParserFactory(std::move(factory));
    builder.PrepareReceiver(receiver);
    return builder.Build();
}

void HandleTask(MailTask *task) {
    assert(task != nullptr);
    sharpen::GenericMail mailWrap{std::move(task->Mail())};
    static std::atomic_uint32_t counter{0};
    std::uint32_t count{counter.fetch_add(1, std::memory_order_relaxed)};
    sharpen::ByteBuffer content{64};
    content.Printf("Hello %u", count);
    task->Channel().WriteAsync(mailWrap.AsMail().Header());
    task->Channel().WriteAsync(mailWrap.AsMail().Content());
}

class BasicTest : public simpletest::ITypenamedTest<BasicTest> {
private:
    using Self = BasicTest;

public:
    BasicTest() noexcept = default;

    ~BasicTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        std::unique_ptr<sharpen::IHost> host{CreateHost(23334, BatchHandlerWrap{1, &HandleTask})};
        sharpen::SingleWorkerGroup work{};
        work.Submit(&sharpen::IHost::Run, host.get());
        EchoReceiver receiver;
        std::unique_ptr<sharpen::IRemoteActor> actor{CreateActor(23334, false, receiver)};
        sharpen::GenericMail mail{magicNumber};
        sharpen::ByteBuffer content{"Hello", 5};
        mail.SetContent(std::move(content));
        actor->Post(mail.ReleaseMail());
        sharpen::Delay(std::chrono::seconds(3));
        actor.reset(nullptr);
        host->Stop();
        work.Stop();
        work.Join();
        return this->Assert(receiver.GetCount() == 1, "Count should == 1");
    }
};

class PipelineTest : public simpletest::ITypenamedTest<PipelineTest> {
private:
    using Self = PipelineTest;

public:
    PipelineTest() noexcept = default;

    ~PipelineTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        std::unique_ptr<sharpen::IHost> host{CreateHost(23333, BatchHandlerWrap{8, &HandleTask})};
        sharpen::SingleWorkerGroup work{};
        work.Submit(&sharpen::IHost::Run, host.get());
        EchoReceiver receiver;
        std::unique_ptr<sharpen::IRemoteActor> actor{CreateActor(23333, true, receiver)};
        for (std::size_t i = 0; i != 32; ++i) {
            sharpen::SyncPrintf("Post %zu\n", i);
            sharpen::GenericMail mail{magicNumber};
            sharpen::ByteBuffer content{"Hello", 5};
            mail.SetContent(std::move(content));
            actor->Post(mail.ReleaseMail());
            sharpen::SyncPrintf("Status %s\n", actor->GetStatusName());
        }
        sharpen::Delay(std::chrono::seconds(3));
        sharpen::SyncPrintf("Status %s\n", actor->GetStatusName());
        actor.reset(nullptr);
        host->Stop();
        work.Stop();
        work.Join();
        return this->Assert(receiver.GetCount() == 32, "Count should == 32");
    }
};

class CancelTest:public simpletest::ITypenamedTest<CancelTest>
{
private:
    using Self = CancelTest;

public:

    CancelTest() noexcept = default;

    ~CancelTest() noexcept = default;

    inline const Self &Const() const noexcept
    {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept
    {
        std::unique_ptr<sharpen::IHost> host{CreateDropHost(23335)};
        sharpen::SingleWorkerGroup work{};
        work.Submit(&sharpen::IHost::Run, host.get());
        EchoReceiver receiver;
        std::unique_ptr<sharpen::IRemoteActor> actor{CreateActor(23335, false, receiver)};
        sharpen::GenericMail mail{magicNumber};
        sharpen::ByteBuffer content{"Hello", 5};
        mail.SetContent(std::move(content));
        actor->Post(mail.ReleaseMail());
        sharpen::Delay(std::chrono::seconds(3));
        sharpen::SyncPrintf("Status %s\n", actor->GetStatusName());
        if (actor->GetStatus() != sharpen::RemoteActorStatus::InProgress) {
            return this->Fail("Status should == inprogress");
        }
        actor->Cancel();
        sharpen::SyncPrintf("Status %s\n", actor->GetStatusName());
        if (actor->GetStatus() != sharpen::RemoteActorStatus::Closed) {
            return this->Fail("Status should == closed");
        }
        actor.reset(nullptr);
        host->Stop();
        work.Stop();
        work.Join();
        return this->Assert(receiver.GetCount() == 0, "Count should == 0");
    }
};

class PipelineCancelTest:public simpletest::ITypenamedTest<PipelineCancelTest>
{
private:
    using Self = PipelineCancelTest;

public:

    PipelineCancelTest() noexcept = default;

    ~PipelineCancelTest() noexcept = default;

    inline const Self &Const() const noexcept
    {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept
    {
        std::unique_ptr<sharpen::IHost> host{CreateDropHost(23336)};
        sharpen::SingleWorkerGroup work{};
        work.Submit(&sharpen::IHost::Run, host.get());
        EchoReceiver receiver;
        std::unique_ptr<sharpen::IRemoteActor> actor{CreateActor(23336, true, receiver)};
        sharpen::GenericMail mail{magicNumber};
        sharpen::ByteBuffer content{"Hello", 5};
        mail.SetContent(std::move(content));
        actor->Post(mail.ReleaseMail());
        for (std::size_t i = 0; i != 32; ++i) {
            sharpen::SyncPrintf("Post %zu\n", i);
            sharpen::GenericMail mail{magicNumber};
            sharpen::ByteBuffer content{"Hello", 5};
            mail.SetContent(std::move(content));
            actor->Post(mail.ReleaseMail());
            sharpen::SyncPrintf("Status %s\n", actor->GetStatusName());
        }
        sharpen::Delay(std::chrono::seconds(3));
        sharpen::SyncPrintf("Status %s\n", actor->GetStatusName());
        if (actor->GetStatus() != sharpen::RemoteActorStatus::InProgress) {
            return this->Fail("Status should == inprogress");
        }
        actor->Cancel();
        sharpen::SyncPrintf("Status %s\n", actor->GetStatusName());
        if (actor->GetStatus() != sharpen::RemoteActorStatus::Closed) {
            return this->Fail("Status should == closed");
        }
        actor.reset(nullptr);
        host->Stop();
        work.Stop();
        work.Join();
        return this->Assert(receiver.GetCount() == 0, "Count should == 0");
    }
};

int Entry() {
    sharpen::StartupNetSupport();
    simpletest::TestRunner runner{simpletest::DisplayMode::Blocked};
    runner.Register<BasicTest>();
    runner.Register<PipelineTest>();
    runner.Register<CancelTest>();
    runner.Register<PipelineCancelTest>();
    int code{runner.Run()};
    sharpen::CleanupNetSupport();
    return code;
}

int main() {
    sharpen::EventEngine &engine{sharpen::EventEngine::SetupEngine()};
    return engine.StartupWithCode(&Entry);
}