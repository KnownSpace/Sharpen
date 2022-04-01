#include <cassert>
#include <cstdio>

#include <sharpen/INetStreamChannel.hpp>
#include <sharpen/IpEndPoint.hpp>
#include <sharpen/EventEngine.hpp>
#include <sharpen/AsyncOps.hpp>

const char data[] = "hello world\n";

void ClientTest();

void ServerTest()
{
    std::printf("server test begin\n");
    sharpen::NetStreamChannelPtr server = sharpen::MakeTcpStreamChannel(sharpen::AddressFamily::Ip);
    sharpen::IpEndPoint serverEndpoint;
    serverEndpoint.SetAddrByString("127.0.0.1");
    serverEndpoint.SetPort(8080);
    server->Bind(serverEndpoint);
    server->Register(sharpen::EventEngine::GetEngine());
    server->Listen(65535);
    sharpen::Launch(&ClientTest);
    sharpen::NetStreamChannelPtr client = server->AcceptAsync();
    client->Register(sharpen::EventEngine::GetEngine());
    sharpen::Size size = client->WriteAsync(data,sizeof(data) - 1);
    assert(size == sizeof(data) - 1);
    sharpen::ByteBuffer buf{4096};
    size = client->ReadAsync(buf);
    assert(size == sizeof(data) - 1);
    for (size_t i = 0; i < size; i++)
    {
        assert(data[i] == buf[i]);
    }
    client->WriteObjectAsync(0);
    std::printf("server test pass\n");
}

void ClientTest()
{
    std::printf("client test begin\n");
    sharpen::NetStreamChannelPtr client = sharpen::MakeTcpStreamChannel(sharpen::AddressFamily::Ip);
    sharpen::IpEndPoint clientEndpoint;
    clientEndpoint.SetAddrByString("127.0.0.1");
    clientEndpoint.SetPort(0);
    client->Bind(clientEndpoint);
    client->Register(sharpen::EventEngine::GetEngine());
    sharpen::IpEndPoint serverEndpoint;
    serverEndpoint.SetAddrByString("127.0.0.1");
    serverEndpoint.SetPort(8080);
    std::printf("client connecting\n");
    client->ConnectAsync(serverEndpoint);
    std::printf("client connected\n");
    sharpen::ByteBuffer buf{4096};
    sharpen::Size size = client->ReadAsync(buf);
    assert(size == sizeof(data) - 1);
    for (size_t i = 0; i < size; i++)
    {
        assert(data[i] == buf[i]);
    }
    size = client->WriteAsync(data,sizeof(data) - 1);
    assert(size == sizeof(data) - 1);
    int val{client->ReadObjectAsync<int>()};
    std::printf("read obj %d\n",val);
    std::printf("client test pass\n");
}

void CancelTest()
{
    sharpen::NetStreamChannelPtr server = sharpen::MakeTcpStreamChannel(sharpen::AddressFamily::Ip);
    sharpen::NetStreamChannelPtr client = sharpen::MakeTcpStreamChannel(sharpen::AddressFamily::Ip);
    sharpen::IpEndPoint addr;
    addr.SetAddrByString("127.0.0.1");
    addr.SetPort(8080);
    server->Bind(addr);
    server->Register(sharpen::EventEngine::GetEngine());
    addr.SetPort(0);
    client->Bind(addr);
    client->Register(sharpen::EventEngine::GetEngine());
    server->Listen(65535);
    std::printf("cancel test begin\n");
    int flag = 0;
    sharpen::AwaitableFuture<sharpen::Size> future[10];
    addr.SetPort(8080);
    client->ConnectAsync(addr);
    char buf[512];
    for (size_t i = 0; i < 10; i++)
    {
        client->ReadAsync(buf,512,future[i]);
    }
    client->Cancel();
    for (size_t i = 0; i < 10; i++)
    {
        try
        {
            future[i].Await();
        }
        catch(const std::system_error& e)
        {
            std::printf("%zu code: %d\nerror: %s\n",i,e.code().value(),e.what());
            flag += 1;
        }
        
    }
    std::printf("cancel test end\n");
    assert(flag == 10);
}

void TimeoutTest()
{
    sharpen::NetStreamChannelPtr server = sharpen::MakeTcpStreamChannel(sharpen::AddressFamily::Ip);
    sharpen::NetStreamChannelPtr client = sharpen::MakeTcpStreamChannel(sharpen::AddressFamily::Ip);
    sharpen::IpEndPoint ep{0,0};
    ep.SetAddrByString("127.0.0.1");
    client->Bind(ep);
    ep.SetPort(8080);
    server->Bind(ep);
    server->Register(sharpen::EventEngine::GetEngine());
    client->Register(sharpen::EventEngine::GetEngine());
    server->Listen(65535);
    sharpen::AwaitableFuture<void> future;
    client->ConnectAsync(ep,future);
    sharpen::NetStreamChannelPtr chd = server->AcceptAsync();
    future.Await();
    chd->Register(sharpen::EventEngine::GetEngine());
    char buf[6] = {};
    sharpen::TimerPtr timer = sharpen::MakeTimer(sharpen::EventEngine::GetEngine());
    auto r = chd->ReadWithTimeout(timer,std::chrono::seconds(1),buf,sizeof(buf));
    if(!r.Exist())
    {
        std::puts("timeout");
    }
}

void NetworkTest()
{
    sharpen::StartupNetSupport();
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    engine.Startup([&engine]()
    {
        std::printf("network test begin\n");
        ServerTest();
        CancelTest();
        TimeoutTest();
        std::printf("network test pass\n");
        sharpen::CleanupNetSupport();
    });
}

int main()
{
    NetworkTest();
    return 0;
}
