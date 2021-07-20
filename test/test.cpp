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
#include <sharpen/HttpRequest.hpp>
#include <sharpen/HttpResponse.hpp>
#include <sharpen/HttpParser.hpp>
#include <sharpen/ProcessInfo.hpp>

void HandleClient(sharpen::NetStreamChannelPtr client)
{
    bool keepalive = true;
    sharpen::ByteBuffer recvBuf(4096);
    sharpen::ByteBuffer sendBuf;
    sharpen::HttpRequest req;
    sharpen::HttpParser parser(sharpen::HttpParser::ParserModel::Request);
    req.ConfigParser(parser);
    sharpen::HttpResponse res(sharpen::HttpVersion::Http1_1,sharpen::HttpStatusCode::OK);
    res.Header()["Connection"] = "keep-alive";
    const char content[] = "hello, world!\n";
    res.Header()["Content-Length"] = std::to_string(sizeof(content) - 1);
    res.Header()["Content-Type"] = "text/plain";
    res.Header()["Server"] = "IIS 6.0";
    res.Body().CopyFrom(content,sizeof(content) - 1);
    res.CopyTo(sendBuf);
    while (keepalive)
    {
       try
       {
            while (!parser.IsCompleted())
            {
                sharpen::Size n = client->ReadAsync(recvBuf);
                if (n == 0)
                {
                    return;
                }
                parser.Parse(recvBuf.Data(),n);
            }
            parser.SetCompleted(false);
            if (!parser.ShouldKeepalive())
            {
                keepalive = false;
                res.Header()["Connection"] = "close";
                res.CopyTo(sendBuf);
            }
            client->WriteAsync(sendBuf);
       }
       catch(const std::exception& e)
       {
           std::printf("handle client error: %s\n",e.what());
           return;
       }
    }
}

void WebTest(sharpen::Size num)
{
    sharpen::StartupNetSupport();
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupEngine(num);
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
    std::printf("run in %u cores machine\nprocess id: %u\n",std::thread::hardware_concurrency(),sharpen::GetProcessId());
    sharpen::Size num = std::thread::hardware_concurrency();
    if (argc > 1)
    {
        num = std::atoi(argv[1]);
    }
    WebTest(num);
    //FileTest();
    return 0;
}