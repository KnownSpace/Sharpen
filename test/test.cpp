#include <cstdio>
#include <mutex>
#include <string>
#include <sharpen/IFileChannel.hpp>
#include <sharpen/EventEngine.hpp>
#include <sharpen/INetStreamChannel.hpp>
#include <sharpen/IpEndPoint.hpp>
#include <sharpen/AsyncOps.hpp>
#include <algorithm>
#include <sharpen/CtrlHandler.hpp>
#include <cinttypes>

#define TEST_COUNT 10000 * 100

void HandleClient(sharpen::NetStreamChannelPtr client)
{
    bool keepalive = true;
    sharpen::ByteBuffer buf(4096);
    std::string close("close");
    while (keepalive)
    {
        try
        {
            //std::printf("read %d\n",count + 1);
            sharpen::Size size = client->ReadAsync(buf);
            //std::printf("new request %zu\n",size);
            if (size == 0)
            {
                keepalive = false;
                break;
            }
            const char data[] = "HTTP/1.1 200\r\nConnection: keep-alive\r\nContent-Length: 2\r\n\r\nOK";
            //std::printf("write %d\n",count + 1);
            size = client->WriteAsync(data, sizeof(data) - 1);
            //std::printf("response completed %zu \n",size);
            if (std::search(buf.Begin(), buf.End(), close.begin(), close.end()) != buf.End())
            {
                keepalive = false;
            }
        }
        catch (const std::exception &e)
        {
            std::printf("error %s\n",e.what());
            keepalive = false;
            break;
        }
    }
    //std::printf("client disconnected\n");
}

void WebTest()
{
    sharpen::StartupNetSupport();
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupEngine(3);
    sharpen::NetStreamChannelPtr server = sharpen::MakeTcpStreamChannel(sharpen::AddressFamily::Ip);
    sharpen::IpEndPoint addr;
    addr.SetAddr("0.0.0.0");
    addr.SetPort(8080);
    server->SetReuseAddress(true);
    server->Bind(addr);
    server->Listen(65535);
    server->Register(engine);
    sharpen::Launch([&server,&engine]()
    {
        while (true)
        {
            try
            {
                sharpen::NetStreamChannelPtr client = server->AcceptAsync();
                client->Register(engine);
                //std::printf("new client connected\n");
                sharpen::Launch(&HandleClient, client);
            }
            catch(const std::exception& e)
            {
                std::printf("error %s\n",e.what());
            }
            
        }
    });
    sharpen::RegisterCtrlHandler(sharpen::CtrlType::Interrupt,[]()
    {
        std::puts("stop now\n");
        sharpen::EventEngine::GetEngine().Stop();
        sharpen::CleanupNetSupport();
    });
    char ip[21] = "";
    addr.GetAddr(ip,sizeof(ip));
    std::printf("now listen on %s:%d\n",ip,addr.GetPort());
    std::printf("use ctrl + c to stop\n");
    engine.Run();
}

int main(int argc, char const *argv[])
{
    WebTest();
    return 0;
}