#include <cstdio>
#include <sharpen/MicroRpcStack.hpp>
#include <sharpen/RpcClient.hpp>
#include <sharpen/INetStreamChannel.hpp>
#include <sharpen/IpEndPoint.hpp>
#include <sharpen/EventEngine.hpp>
#include <sharpen/MicroRpcEncoder.hpp>
#include <sharpen/MicroRpcDecoder.hpp>
#include <sharpen/MicroRpcDispatcher.hpp>
#include <sharpen/RpcServer.hpp>
#include <sharpen/CtrlHandler.hpp>

using MicroRpcServer = sharpen::RpcServer<sharpen::MicroRpcStack,sharpen::MicroRpcEncoder,sharpen::MicroRpcStack,sharpen::MicroRpcDecoder,sharpen::MicroRpcDispatcher>;

using Option = typename MicroRpcServer::Option;

using Context = typename MicroRpcServer::Context;

void Entry()
{
    sharpen::StartupNetSupport();
    sharpen::EventEngine &engine = sharpen::EventEngine::GetEngine();
    sharpen::IpEndPoint addr;
    addr.SetAddrByString("127.0.0.1");
    addr.SetPort(8082);
    Option opt(sharpen::MicroRpcDispatcher{},std::chrono::seconds(3));
    MicroRpcServer server(sharpen::AddressFamily::Ip,addr,sharpen::EventEngine::GetEngine(),std::move(opt));
    server.RegisterTimeout([](Context &ctx)
    {
        sharpen::IpEndPoint addr;
        ctx.Connection()->GetRemoteEndPoint(addr);
        char ip[21] = {0};
        addr.GetAddrSring(ip,sizeof(ip));
        std::printf("%s:%u timeout disconnect\n",ip,addr.GetPort());
    });
    server.Register("Hello",[](Context &ctx)
    {
        sharpen::MicroRpcStack res;
        res.Push(std::rand());
        ctx.Connection()->WriteAsync(ctx.Encoder().Encode(res));
    });
    sharpen::RegisterCtrlHandler(sharpen::CtrlType::Interrupt,[&server]() mutable
    {
        std::printf("stop now\n");
        server.Stop();
    });
    server.RunAsync();
    sharpen::CleanupNetSupport();
}

int main(int argc, char const *argv[])
{
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupEngine();
    engine.Startup(&Entry);
    return 0;
}