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
    std::printf("client test pass\n");
}

void NetworkTest()
{
    sharpen::StartupNetSupport();
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupEngine();
    engine.Startup([&engine]()
    {
        std::printf("network test begin\n");
        ServerTest();
        std::printf("network test pass\n");
        sharpen::CleanupNetSupport();
    });
}

int main()
{
    NetworkTest();
    return 0;
}
