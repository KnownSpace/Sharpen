#include <cstdio>
#include <sharpen/IpEndPoint.hpp>
#include <sharpen/INetStreamChannel.hpp>
#include <sharpen/EventEngine.hpp>

void Entry()
{
    sharpen::StartupNetSupport();
    sharpen::IpEndPoint addr;
    addr.SetAddrByString("127.0.0.1");
    addr.SetPort(0);
    sharpen::NetStreamChannelPtr chann = sharpen::MakeTcpStreamChannel(sharpen::AddressFamily::Ip);
    chann->Bind(addr);
    chann->Register(sharpen::EventEngine::GetEngine());
    chann->GetLocalEndPoint(addr);
    std::printf("bind on %u\n",addr.GetPort());
    chann->ConnectAsync(addr);
    std::puts("connect success");
    chann->WriteAsync("hello self",10);
    char buf[11] = {0};
    chann->ReadAsync(buf,sizeof(buf));
    std::puts(buf);
    sharpen::CleanupNetSupport();
}

int main(int argc, char const *argv[])
{
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    engine.Startup(&Entry);
    return 0;
}
