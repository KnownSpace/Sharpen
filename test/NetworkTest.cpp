#include <cassert>
#include <cstdio>

#include <sharpen/INetStreamChannel.hpp>
#include <sharpen/IpEndPoint.hpp>
#include <sharpen/EventEngine.hpp>
#include <sharpen/AsyncOps.hpp>
#include <sharpen/TcpHost.hpp>
#include <sharpen/SimpleHostPipeline.hpp>
#include <sharpen/IpTcpStreamFactory.hpp>
#include <sharpen/YieldOps.hpp>
#include <sharpen/TimerOps.hpp>

#include <simpletest/TestRunner.hpp>

static const char data[] = "hello world\n";

class PingpoingTest :public simpletest::ITypenamedTest<PingpoingTest>
{
private:
    using Self = PingpoingTest;

public:

    PingpoingTest() noexcept = default;

    ~PingpoingTest() noexcept = default;

    inline const Self &Const() const noexcept
    {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept
    {
        sharpen::NetStreamChannelPtr server = sharpen::OpenTcpStreamChannel(sharpen::AddressFamily::Ip);
        sharpen::IpEndPoint serverEndpoint;
        serverEndpoint.SetAddrByString("127.0.0.1");
        serverEndpoint.SetPort(8080);
        server->Bind(serverEndpoint);
        server->Register(sharpen::GetLocalLoopGroup());
        server->Listen(65535);
        sharpen::NetStreamChannelPtr client = sharpen::OpenTcpStreamChannel(sharpen::AddressFamily::Ip);
        sharpen::IpEndPoint clientEndpoint;
        clientEndpoint.SetAddrByString("127.0.0.1");
        clientEndpoint.SetPort(0);
        client->Bind(clientEndpoint);
        client->Register(sharpen::GetLocalLoopGroup());
        auto future = sharpen::Async([&serverEndpoint,client]() mutable
        {
            client->ConnectAsync(serverEndpoint);
            client->WriteAsync(data,sizeof(data) - 1);
        });
        char buf[sizeof(data)] = {0};
        client = server->AcceptAsync();
        client->Register(sharpen::GetLocalLoopGroup());
        client->ReadAsync(buf,sizeof(buf));
        future->Await();
        return this->Assert(!std::strncmp(buf,data,sizeof(data) - 1),"buf should == data,but it not");
    }
};

class CancelTest :public simpletest::ITypenamedTest<CancelTest>
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
        sharpen::NetStreamChannelPtr server = sharpen::OpenTcpStreamChannel(sharpen::AddressFamily::Ip);
        sharpen::NetStreamChannelPtr client = sharpen::OpenTcpStreamChannel(sharpen::AddressFamily::Ip);
        sharpen::IpEndPoint addr;
        addr.SetAddrByString("127.0.0.1");
        addr.SetPort(8081);
        server->Bind(addr);
        server->Register(sharpen::GetLocalLoopGroup());
        addr.SetPort(0);
        client->Bind(addr);
        client->Register(sharpen::GetLocalLoopGroup());
        server->Listen(65535);
        sharpen::AwaitableFuture<std::size_t> future[10];
        addr.SetPort(8081);
        client->ConnectAsync(addr);
        char buf[512] = {0};
        for(std::size_t i = 0;i != sizeof(future)/sizeof(*future);++i)
        {
            client->ReadAsync(buf,512,future[i]);
        }
        client->Cancel();
        bool status{true};
        for(std::size_t i = 0;i != sizeof(future)/sizeof(*future);++i)
        {
            try
            {
                future[i].Await();
            }
            catch(const std::system_error &e)
            {
                if(e.code().value() != sharpen::ErrorCancel)
                {
                    status = false;
                }
            }
        }
        return this->Assert(status,"All operations should throw ErrorCancel,but it not");
    }
};

class TimeoutTest:public simpletest::ITypenamedTest<TimeoutTest>
{
private:
    using Self = TimeoutTest;

public:

    TimeoutTest() noexcept = default;

    ~TimeoutTest() noexcept = default;

    inline const Self &Const() const noexcept
    {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept
    {
        sharpen::NetStreamChannelPtr server = sharpen::OpenTcpStreamChannel(sharpen::AddressFamily::Ip);
        sharpen::NetStreamChannelPtr client = sharpen::OpenTcpStreamChannel(sharpen::AddressFamily::Ip);
        sharpen::IpEndPoint ep{0,0};
        ep.SetAddrByString("127.0.0.1");
        client->Bind(ep);
        ep.SetPort(8082);
        server->Bind(ep);
        server->Register(sharpen::GetLocalLoopGroup());
        client->Register(sharpen::GetLocalLoopGroup());
        server->Listen(65535);
        sharpen::AwaitableFuture<void> future;
        client->ConnectAsync(ep,future);
        sharpen::NetStreamChannelPtr chd = server->AcceptAsync();
        future.Await();
        chd->Register(sharpen::GetLocalLoopGroup());
        char buf[6] = {};
        sharpen::TimerPtr timer = sharpen::MakeTimer(sharpen::GetLocalLoopGroup());
        auto r = chd->ReadWithTimeout(timer,std::chrono::seconds(1),buf,sizeof(buf));
        return this->Assert(!r.Exist(),"r should not exist,but it exist");
    }
};

class CloseTest:public simpletest::ITypenamedTest<CloseTest>
{
private:
    using Self = CloseTest;

public:

    CloseTest() noexcept = default;

    ~CloseTest() noexcept = default;

    inline const Self &Const() const noexcept
    {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept
    {
        sharpen::NetStreamChannelPtr server = sharpen::OpenTcpStreamChannel(sharpen::AddressFamily::Ip);
        sharpen::NetStreamChannelPtr client = sharpen::OpenTcpStreamChannel(sharpen::AddressFamily::Ip);
        sharpen::IpEndPoint ep{0,0};
        ep.SetAddrByString("127.0.0.1");
        client->Bind(ep);
        ep.SetPort(8083);
        server->Bind(ep);
        server->Register(sharpen::GetLocalLoopGroup());
        client->Register(sharpen::GetLocalLoopGroup());
        server->Listen(65535);
        sharpen::AwaitableFuture<void> future;
        client->ConnectAsync(ep,future);
        sharpen::NetStreamChannelPtr chd = server->AcceptAsync();
        future.Await();
        chd->Register(sharpen::GetLocalLoopGroup());
        char buf[6] = {0};
        sharpen::Launch([chd]()
        {
            sharpen::TimerPtr timer = sharpen::MakeTimer(sharpen::GetLocalLoopGroup());
            timer->Await(std::chrono::seconds(3));
            chd->Close();
        });
        bool status{false};
        try
        {
            chd->ReadAsync(buf,sizeof(buf));
        }
        catch(const std::system_error &e)
        {
            status = e.code().value() == sharpen::ErrorConnectionAborted;
        }
        return this->Assert(status,"should throw ErrorConnectionAborted,but it not");
    }
};

class SimpleStep:public sharpen::IHostPipelineStep
{
private:
    using Self = SimpleStep;
protected:
public:

    sharpen::ByteBuffer *data_;

    SimpleStep() noexcept = default;

    SimpleStep(const Self &other) noexcept = default;

    SimpleStep(Self &&other) noexcept = default;

    Self &operator=(const Self &other) noexcept = default;

    Self &operator=(Self &&other) noexcept = default;

    ~SimpleStep() noexcept = default;

    inline const Self &Const() const noexcept
    {
        return *this;
    }

    inline virtual sharpen::HostPipelineResult Consume(sharpen::INetStreamChannel &channel,const std::atomic_bool &active) noexcept override
    {
        if(active)
        {
            //process data
            sharpen::Delay(std::chrono::seconds(2));
            channel.ReadAsync(*this->data_);
        }
        return sharpen::HostPipelineResult::Broken;
    }
};

static std::unique_ptr<sharpen::IHostPipeline> ConfiguratePipeline(SimpleStep *step)
{
    std::unique_ptr<sharpen::IHostPipeline> pipeline{new (std::nothrow) sharpen::SimpleHostPipeline{}};
    pipeline->RegisterStep(std::unique_ptr<sharpen::IHostPipelineStep>{step});
    return pipeline;
}

class HostTest:public simpletest::ITypenamedTest<HostTest>
{
private:
    using Self = HostTest;

public:

    HostTest() noexcept = default;

    ~HostTest() noexcept = default;

    inline const Self &Const() const noexcept
    {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept
    {
        sharpen::IpEndPoint endPoint;
        endPoint.SetAddrByString("127.0.0.1");
        endPoint.SetPort(8084);
        sharpen::IpTcpStreamFactory factory{sharpen::GetLocalLoopGroup(),endPoint};
        sharpen::TcpHost host{sharpen::GetLocalScheduler(),factory};
        sharpen::ByteBuffer receivedData{4096};
        std::unique_ptr<SimpleStep> step{new (std::nothrow) SimpleStep{}};
        step->data_ = &receivedData;
        host.ConfiguratePipeline(&ConfiguratePipeline,step.release());
        auto future = sharpen::Async([&host]() mutable
        {
            host.Run();
        });
        //let host runs
        sharpen::YieldCycle();
        sharpen::NetStreamChannelPtr client = sharpen::OpenTcpStreamChannel(sharpen::AddressFamily::Ip);
        endPoint.SetPort(0);
        client->Bind(endPoint);
        client->Register(sharpen::GetLocalLoopGroup());
        endPoint.SetPort(8084);
        client->ConnectAsync(endPoint);
        client->WriteAsync(data,sizeof(data) - 1);
        client->Close();
        sharpen::YieldCycle();
        host.Stop();
        if(!future->IsPending())
        {
            return this->Fail("future should still pending,but completed");
        }
        future->WaitAsync();
        return this->Assert(!std::strncmp(data,receivedData.Data(),sizeof(data) - 1),"Pingpong failed");
    }
};

static int Test()
{
    sharpen::StartupNetSupport();
    simpletest::TestRunner runner;
    runner.Register<PingpoingTest>();
    runner.Register<CancelTest>();
    runner.Register<TimeoutTest>();
    runner.Register<CloseTest>();
    runner.Register<HostTest>();
    int code{runner.Run()};
    sharpen::CleanupNetSupport();
    return code;
}

int main()
{
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    return engine.StartupWithCode(&Test);
}
