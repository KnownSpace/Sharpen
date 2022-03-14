#include <cstdio>
#include <sharpen/INetStreamChannel.hpp>
#include <sharpen/IpEndPoint.hpp>
#include <sharpen/EventEngine.hpp>
#include <sharpen/Converter.hpp>
#include <sharpen/IInputPipeChannel.hpp>
#include <sharpen/AsyncOps.hpp>

void Entry(const char *ip,sharpen::UintPort port)
{
    sharpen::StartupNetSupport();
    sharpen::NetStreamChannelPtr channel = sharpen::MakeTcpStreamChannel(sharpen::AddressFamily::Ip);
    sharpen::IpEndPoint endpoint;
    endpoint.SetPort(0);
    endpoint.SetAddrByString("0.0.0.0");
    channel->Bind(endpoint);
    sharpen::InputPipeChannelPtr input = sharpen::MakeStdinPipe();
    channel->Register(sharpen::EventEngine::GetEngine());
    input->Register(sharpen::EventEngine::GetEngine());
    endpoint.SetPort(port);
    try
    {
        endpoint.SetAddrByString(ip);
        channel->ConnectAsync(endpoint);
    }
    catch(const std::exception& e)
    {
        std::fprintf(stderr,"%s\n",e.what());
        return;
    }
    std::puts("connected\nuse ctrl+c to exit");
    std::atomic_bool token{false};
    auto recvLoop = sharpen::Async([&token,channel,input]()
    {
        sharpen::ByteBuffer buf{4096};
        while (!token)
        {
            try
            {
                sharpen::Size size = channel->ReadAsync(buf);
                if(size == 0)
                {
                    token = true;
                    channel->Cancel();
                    input->Close();
                    return;
                }
                for (sharpen::Size i = 0; i < size; ++i)
                {
                    std::putchar(buf[i]);
                }
            }
            catch(const std::exception& e)
            {
                std::fprintf(stderr,"%s\n",e.what());
                token = true;
            }
        }
    });
    sharpen::ByteBuffer buf{4096};
    while (!token)
    {
        try
        {
            sharpen::Size size = input->ReadAsync(buf);
            channel->WriteAsync(buf.Data(),size);
        }
        catch(const std::exception& e)
        {
            std::fprintf(stderr,"%s\n",e.what());
            token = true;
        }
    }
    recvLoop->Await();
    sharpen::CleanupNetSupport();
}

void PrintUsage()
{
    std::puts("usage: <ip> <port> - connect to ip:port");
}

int main(int argc, char const *argv[])
{
    if(argc < 3)
    {
        PrintUsage();
        return 0;
    }
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    sharpen::UintPort port{0};
    try
    {
        port = sharpen::Atoi<sharpen::UintPort>(argv[2],std::strlen(argv[2]));
    }
    catch(const std::exception& e)
    {
        std::fprintf(stderr,"%s\n",e.what());
        return -1;
    }
    engine.Startup(&Entry,argv[1],port);   
    return 0;
}