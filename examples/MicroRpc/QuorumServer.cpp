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
#include <sharpen/Converter.hpp>
#include <sharpen/AsyncBarrier.hpp>
#include <sharpen/AsyncOps.hpp>

using MicroRpcServer = sharpen::RpcServer<sharpen::MicroRpcStack,sharpen::MicroRpcEncoder,sharpen::MicroRpcStack,sharpen::MicroRpcDecoder,sharpen::MicroRpcDispatcher>;

using Option = typename MicroRpcServer::Option;

using Context = typename MicroRpcServer::Context;

void Entry()
{
    sharpen::StartupNetSupport();
    sharpen::EventEngine &engine = sharpen::EventEngine::GetEngine();
    sharpen::IpEndPoint addr;
    addr.SetAddrByString("127.0.0.1");
    Option opt(sharpen::MicroRpcDispatcher{},std::chrono::seconds(3));
    std::vector<std::unique_ptr<MicroRpcServer>> servers;
    for (size_t i = 0; i < 3; i++)
    {
        addr.SetPort(static_cast<sharpen::UintPort>(8080 + i));
        servers.emplace_back(new MicroRpcServer{sharpen::AddressFamily::Ip,addr,sharpen::EventEngine::GetEngine(),opt});
        servers.back()->RegisterTimeout([](Context &ctx)
        {
            sharpen::IpEndPoint addr;
            ctx.Connection()->GetRemoteEndPoint(addr);
            char ip[21] = {0};
            addr.GetAddrString(ip,sizeof(ip));
            std::printf("%s:%u timeout disconnect\n",ip,addr.GetPort());
        });
        servers.back()->Register("Hello",[](Context &ctx)
        {
            std::printf("recv req\n");
            sharpen::MicroRpcStack res;
            res.Push(std::rand());
            ctx.Connection()->WriteAsync(ctx.Encoder().Encode(res));
        });
    }
    
    sharpen::RegisterCtrlHandler(sharpen::CtrlType::Interrupt,[&servers]() mutable
    {
        for (auto begin = servers.begin(),end = servers.end();begin != end;++begin)
        {
            (*begin)->Stop();
        }
    });
    sharpen::AsyncBarrier barrier(servers.size());
    for (auto begin = servers.begin(),end = servers.end();begin != end;++begin)
    {
        MicroRpcServer &server = **begin;
        sharpen::Launch([&server,&barrier]()
        {
            server.RunAsync();
            barrier.Notice();
        });
    }
    barrier.WaitAsync();
    std::puts("application stop");
    sharpen::CleanupNetSupport();
}

int main(int argc, char const *argv[])
{
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupEngine();
    engine.Startup(&Entry);
    return 0;
}