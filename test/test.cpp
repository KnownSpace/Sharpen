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
#include <cstring>

#define TEST_COUNT 10000 * 100

void HandleClient(sharpen::NetStreamChannelPtr client)
{
    bool keepalive = true;
    sharpen::ByteBuffer buf(4096);
    while (keepalive)
    {
        try
        {
            sharpen::Size size = client->ReadAsync(buf);
            if (size == 0)
            {
                keepalive = false;
                break;
            }
            const char data[] = "HTTP/1.1 200\r\nConnection: keep-alive\r\nContent-Length: 2\r\n\r\nOK";
            size = client->WriteAsync(data, sizeof(data) - 1);
        }
        catch (const std::exception &e)
        {
            std::printf("handle client error %s\n",e.what());
            keepalive = false;
            break;
        }
    }
}

void WebTest()
{
    sharpen::StartupNetSupport();
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupEngine();
    sharpen::NetStreamChannelPtr server = sharpen::MakeTcpStreamChannel(sharpen::AddressFamily::Ip);
    sharpen::IpEndPoint addr;
    addr.SetAddrByString("0.0.0.0");
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
                sharpen::Launch(&HandleClient, client);
            }
            catch(const std::system_error &e)
            {
                std::printf("accept error code %d msg %s",e.code().value(),e.what());
            }
            catch(const std::exception& e)
            {
                std::printf("accept error %s\n",e.what());
                //break;
            }
        }
    });
    sharpen::RegisterCtrlHandler(sharpen::CtrlType::Interrupt,[]()
    {
        std::puts("stop now\n");
        sharpen::EventEngine::GetEngine().Stop();
        sharpen::CleanupNetSupport();
    });
    char ip[21];
    std::memset(ip,0,sizeof(ip));
    addr.GetAddrSring(ip,sizeof(ip));
    std::printf("now listen on %s:%d\n",ip,addr.GetPort());
    std::printf("use ctrl + c to stop\n");
    engine.Run();
}

void FileTest()
{
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupEngine(1);
    sharpen::Launch([&engine]()
    {
        sharpen::FileChannelPtr channel = sharpen::MakeFileChannel("./newFile.txt",sharpen::FileAccessModel::All,sharpen::FileOpenModel::CreateOrOpen);
        channel->Register(engine);
        sharpen::Launch([]()
        {
            std::printf("writing\n");
        });
        sharpen::ByteBuffer buf("hello world",11);
        std::printf("prepare write operation\n");
        //await in here
        channel->WriteAsync(buf,0);
        std::printf("write done\n");
        engine.Stop();
    });
    engine.Run();
}

int main(int argc, char const *argv[])
{
    WebTest();
    //FileTest();
    return 0;
}