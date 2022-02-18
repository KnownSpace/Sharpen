#include <cstdio>
#include <string>
#include <sharpen/EventEngine.hpp>
#include <sharpen/INetStreamChannel.hpp>
#include <sharpen/IpEndPoint.hpp>
#include <sharpen/CtrlHandler.hpp>
#include <cstring>
#include <sharpen/ProcessInfo.hpp>
#include <sharpen/HttpServer.hpp>
#include <sharpen/Converter.hpp>

class TestHttpServer:public sharpen::HttpServer
{
private:
protected:
    void OnNewMessage(sharpen::NetStreamChannelPtr channel,const sharpen::HttpRequest &req,sharpen::HttpResponse &res) override
    {
        const char content[] = "hello world";
        res.Header()["Content-Type"] = "text/plain";
        res.Header()["Content-Length"] = std::to_string(sizeof(content) - 1);
        res.Body().CopyFrom(content,sizeof(content) - 1);
    }
public:
    explicit TestHttpServer(const sharpen::IEndPoint &endpoint)
        :sharpen::HttpServer(sharpen::AddressFamily::Ip,endpoint,sharpen::EventEngine::GetEngine(),"IIS/Linux")
    {}

    ~TestHttpServer() noexcept = default;
};

void Entry()
{
    sharpen::StartupNetSupport();
    sharpen::IpEndPoint addr;
    addr.SetAddrByString("0.0.0.0");
    addr.SetPort(8080);
    TestHttpServer server(addr);
    char ip[21];
    std::memset(ip,0,sizeof(ip));
    addr.GetAddrString(ip,sizeof(ip));
    std::printf("now listen on %s:%d\n",ip,addr.GetPort());
    std::printf("use ctrl + c to stop\n");
    sharpen::RegisterCtrlHandler(sharpen::CtrlType::Interrupt,[&server]() mutable
    {
        server.Stop();
    });
    server.RunAsync();
    std::puts("server stop");
    sharpen::CleanupNetSupport();
}

int main(int argc, char const *argv[])
{
    std::printf("run in %u cores machine\nprocess id: %u\n",std::thread::hardware_concurrency(),sharpen::GetProcessId());
    sharpen::Size num = std::thread::hardware_concurrency();
    if (argc > 1)
    {
        num = sharpen::Atoi<sharpen::Size>(argv[1],std::strlen(argv[1]));
        if(num == 0)
        {
            return -1;
        }
    }
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupEngine(num);
    engine.Startup(&Entry);
}