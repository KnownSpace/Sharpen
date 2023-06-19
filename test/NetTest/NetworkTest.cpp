#include <sharpen/AsyncOps.hpp>
#include <sharpen/DebugTools.hpp>
#include <sharpen/EventEngine.hpp>
#include <sharpen/INetStreamChannel.hpp>
#include <sharpen/IpEndPoint.hpp>
#include <sharpen/IpTcpStreamFactory.hpp>
#include <sharpen/SimpleHostPipeline.hpp>
#include <sharpen/TcpHost.hpp>
#include <sharpen/TimerOps.hpp>
#include <sharpen/YieldOps.hpp>
#include <simpletest/TestRunner.hpp>
#include <cassert>
#include <cstdio>


static const char data[] = "hello world\n";

static const std::uint16_t testPort{10808};

class PingpoingTest : public simpletest::ITypenamedTest<PingpoingTest> {
private:
    using Self = PingpoingTest;

public:
    PingpoingTest() noexcept = default;

    ~PingpoingTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        sharpen::NetStreamChannelPtr server = sharpen::OpenTcpChannel(sharpen::AddressFamily::Ip);
        sharpen::IpEndPoint serverEndpoint;
        serverEndpoint.SetAddrByString("127.0.0.1");
        serverEndpoint.SetPort(testPort);
        server->ReuseAddressInNix();
        server->Bind(serverEndpoint);
        server->Register(sharpen::GetLocalLoopGroup());
        server->Listen(65535);
        sharpen::NetStreamChannelPtr client = sharpen::OpenTcpChannel(sharpen::AddressFamily::Ip);
        sharpen::IpEndPoint clientEndpoint;
        clientEndpoint.SetAddrByString("127.0.0.1");
        clientEndpoint.SetPort(0);
        client->Bind(clientEndpoint);
        client->Register(sharpen::GetLocalLoopGroup());
        auto future = sharpen::Async([&serverEndpoint, client]() mutable {
            client->ConnectAsync(serverEndpoint);
            client->WriteAsync(data, sizeof(data) - 1);
        });
        char buf[sizeof(data)] = {0};
        client = server->AcceptAsync();
        client->Register(sharpen::GetLocalLoopGroup());
        client->ReadAsync(buf, sizeof(buf));
        future->Await();
        return this->Assert(!std::strncmp(buf, data, sizeof(data) - 1),
                            "buf should == data,but it not");
    }
};

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
        sharpen::NetStreamChannelPtr server = sharpen::OpenTcpChannel(sharpen::AddressFamily::Ip);
        sharpen::NetStreamChannelPtr client = sharpen::OpenTcpChannel(sharpen::AddressFamily::Ip);
        sharpen::IpEndPoint addr;
        addr.SetAddrByString("127.0.0.1");
        std::uint16_t port{testPort};
        addr.SetPort(port);
        server->ReuseAddressInNix();
        server->Bind(addr);
        server->Register(sharpen::GetLocalLoopGroup());
        addr.SetPort(0);
        client->Bind(addr);
        client->Register(sharpen::GetLocalLoopGroup());
        server->Listen(65535);
        sharpen::AwaitableFuture<std::size_t> future[10];
        addr.SetPort(port);
        client->ConnectAsync(addr);
        char buf[512] = {0};
        for (std::size_t i = 0; i != sizeof(future) / sizeof(*future); ++i) {
            client->ReadAsync(buf, 512, future[i]);
        }
        client->Cancel();
        bool status{true};
        for (std::size_t i = 0; i != sizeof(future) / sizeof(*future); ++i) {
            std::size_t size{future[i].Await()};
            if (size) {
                status = false;
            }
        }
        return this->Assert(status, "All operations should return 0,but it not");
    }
};

class TimeoutTest : public simpletest::ITypenamedTest<TimeoutTest> {
private:
    using Self = TimeoutTest;

public:
    TimeoutTest() noexcept = default;

    ~TimeoutTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        sharpen::NetStreamChannelPtr server = sharpen::OpenTcpChannel(sharpen::AddressFamily::Ip);
        sharpen::NetStreamChannelPtr client = sharpen::OpenTcpChannel(sharpen::AddressFamily::Ip);
        sharpen::IpEndPoint ep{0, 0};
        ep.SetAddrByString("127.0.0.1");
        client->Bind(ep);
        ep.SetPort(testPort);
        server->ReuseAddressInNix();
        server->Bind(ep);
        server->Register(sharpen::GetLocalLoopGroup());
        client->Register(sharpen::GetLocalLoopGroup());
        server->Listen(65535);
        sharpen::AwaitableFuture<void> future;
        client->ConnectAsync(ep, future);
        sharpen::NetStreamChannelPtr chd = server->AcceptAsync();
        future.Await();
        chd->Register(sharpen::GetLocalLoopGroup());
        char buf[6] = {};
        sharpen::TimerPtr timer = sharpen::MakeTimer(sharpen::GetLocalLoopGroup());
        auto r = chd->ReadWithTimeout(timer, std::chrono::seconds(1), buf, sizeof(buf));
        return this->Assert(r.Get() == 0, "r should == 0,but it not");
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
        sharpen::NetStreamChannelPtr server = sharpen::OpenTcpChannel(sharpen::AddressFamily::Ip);
        sharpen::NetStreamChannelPtr client = sharpen::OpenTcpChannel(sharpen::AddressFamily::Ip);
        sharpen::IpEndPoint ep{0, 0};
        ep.SetAddrByString("127.0.0.1");
        client->Bind(ep);
        ep.SetPort(testPort);
        server->ReuseAddressInNix();
        server->Bind(ep);
        server->Register(sharpen::GetLocalLoopGroup());
        client->Register(sharpen::GetLocalLoopGroup());
        server->Listen(65535);
        sharpen::AwaitableFuture<void> future;
        client->ConnectAsync(ep, future);
        sharpen::NetStreamChannelPtr chd = server->AcceptAsync();
        future.Await();
        chd->Register(sharpen::GetLocalLoopGroup());
        char buf[6] = {0};
        sharpen::Launch([chd]() {
            sharpen::TimerPtr timer = sharpen::MakeTimer(sharpen::GetLocalLoopGroup());
            timer->Await(std::chrono::seconds(3));
            chd->Close();
        });
        std::size_t size{chd->ReadAsync(buf, sizeof(buf))};
        return this->Assert(size == 0, "size should == 0,but it not");
    }
};

class HalfAcceptTest : public simpletest::ITypenamedTest<HalfAcceptTest> {
private:
    using Self = HalfAcceptTest;

public:
    HalfAcceptTest() noexcept = default;

    ~HalfAcceptTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        sharpen::NetStreamChannelPtr server = sharpen::OpenTcpChannel(sharpen::AddressFamily::Ip);
        sharpen::NetStreamChannelPtr client = sharpen::OpenTcpChannel(sharpen::AddressFamily::Ip);
        sharpen::IpEndPoint ep{0, 0};
        ep.SetAddrByString("127.0.0.1");
        client->Bind(ep);
        ep.SetPort(testPort);
        server->ReuseAddressInNix();
        server->Bind(ep);
        server->Register(sharpen::GetLocalLoopGroup());
        client->Register(sharpen::GetLocalLoopGroup());
        server->Listen(65535);
        client->ConnectAsync(ep);
        sharpen::ByteBuffer buf{"Hello", 5};
        client->WriteAsync(buf);
        client->Close();
        sharpen::NetStreamChannelPtr conn{nullptr};
        try {
            conn = server->AcceptAsync();
        } catch (const std::system_error &e) {
            sharpen::ErrorCode err{sharpen::GetErrorCode(e)};
            return this->Assert(err == sharpen::ErrorConnectionReset,"error should be connection reset");
        }
        conn->Register(sharpen::GetLocalLoopGroup());
        std::size_t sz{conn->ReadAsync(buf)};
        while (sz != 0) {
            sz = conn->ReadAsync(buf);
        }
        return this->Success();
    }
};

static int Test() {
    sharpen::StartupNetSupport();
    simpletest::TestRunner runner;
    runner.Register<PingpoingTest>();
    runner.Register<CancelTest>();
    runner.Register<TimeoutTest>();
    runner.Register<CloseTest>();
    runner.Register<HalfAcceptTest>();
    int code{runner.Run()};
    sharpen::CleanupNetSupport();
    return code;
}

int main() {
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    return engine.StartupWithCode(&Test);
}
