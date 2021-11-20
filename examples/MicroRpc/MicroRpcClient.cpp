#include <cstdio>
#include <sharpen/MicroRpcStack.hpp>
#include <sharpen/RpcClient.hpp>
#include <sharpen/INetStreamChannel.hpp>
#include <sharpen/IpEndPoint.hpp>
#include <sharpen/EventEngine.hpp>
#include <sharpen/MicroRpcEncoder.hpp>
#include <sharpen/MicroRpcDecoder.hpp>

using MicroRpcClient = sharpen::RpcClient<sharpen::MicroRpcStack,sharpen::MicroRpcEncoder,sharpen::MicroRpcStack,sharpen::MicroRpcDecoder>;

void Entry()
{
    sharpen::StartupNetSupport();
    sharpen::EventEngine &engine = sharpen::EventEngine::GetEngine();
    sharpen::IpEndPoint addr;
    addr.SetAddrByString("127.0.0.1");
    addr.SetPort(0);
    sharpen::NetStreamChannelPtr conn = sharpen::MakeTcpStreamChannel(sharpen::AddressFamily::Ip);
    conn->Bind(addr);
    conn->Register(engine);
    addr.SetPort(8082);
    conn->ConnectAsync(addr);
    MicroRpcClient client(conn);
    sharpen::MicroRpcStack req;
    char proc[] = "Hello";
    req.Push(1);
    req.Push(proc,proc + sizeof(proc) - 1);
    sharpen::MicroRpcStack res = client.InvokeAsync(req);
    int *data = res.Top().Data<int>();
    std::printf("return value is %d",*data);
    sharpen::CleanupNetSupport();
}

int main(int argc, char const *argv[])
{
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupEngine();
    engine.Startup(&Entry);
    return 0;
}
