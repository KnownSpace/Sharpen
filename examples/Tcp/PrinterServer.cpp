#include <cstdio>
#include <string>
#include <sharpen/EventEngine.hpp>
#include <sharpen/INetStreamChannel.hpp>
#include <sharpen/IpEndPoint.hpp>
#include <sharpen/CtrlHandler.hpp>
#include <sharpen/TcpServer.hpp>
#include <sharpen/Converter.hpp>

class PrinterServer:public sharpen::TcpServer
{
private:
    virtual void OnNewChannel(sharpen::NetStreamChannelPtr channel) override
    {
        sharpen::ByteBuffer buf{4096};
        while (this->IsRunning())
        {
            sharpen::Size size = channel->ReadAsync(buf);
            if (!size)
            {
                return;
            }
            for (sharpen::Size i = 0; i < size; ++i)
            {
                std::putchar(buf[i]);
            }
        }
    }

public:
    PrinterServer(sharpen::AddressFamily af,const sharpen::IEndPoint &endpoint,sharpen::EventEngine &engine)
        :TcpServer(af,endpoint,engine)
    {}

    virtual ~PrinterServer() noexcept = default;
};

void Entry(sharpen::UintPort port)
{
    sharpen::StartupNetSupport();
    sharpen::IpEndPoint addr;
    addr.SetAddrByString("0.0.0.0");
    addr.SetPort(port);
    PrinterServer server{sharpen::AddressFamily::Ip,addr,sharpen::EventEngine::GetEngine()};
    char ip[21];
    std::memset(ip,0,sizeof(ip));
    addr.GetAddrString(ip,sizeof(ip));
    std::printf("now listen on %s:%d\n",ip,addr.GetPort());
    std::puts("use ctrl + c to stop");
    server.RunAsync();
    sharpen::CleanupNetSupport();
}

void PrintUsage()
{
    std::puts("usage: <port> - listen on <port>");
}

int main(int argc, char const *argv[])
{
    if(argc < 2)
    {
        PrintUsage();
        return 0;
    }
    sharpen::UintPort port{0};
    try
    {
        port = sharpen::Atoi<sharpen::UintPort>(argv[1],std::strlen(argv[1]));
    }
    catch(const std::exception& e)
    {
        std::fprintf(stderr,"%s\n",e.what());
        return -1;
    }
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    engine.Startup(&Entry,port);
}