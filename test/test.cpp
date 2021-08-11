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
#include <sharpen/HttpServer.hpp>
#include <sharpen/TimeWheel.hpp>
#include <sharpen/AwaitOps.hpp>

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

void WebTest(sharpen::Size num)
{
    sharpen::StartupNetSupport();
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupEngine(num);
    sharpen::IpEndPoint addr;
    addr.SetAddrByString("0.0.0.0");
    addr.SetPort(8080);
    TestHttpServer server(addr);
    sharpen::RegisterCtrlHandler(sharpen::CtrlType::Interrupt,[]()
    {
        std::puts("stop now\n");
        sharpen::EventEngine::GetEngine().Stop();
        std::puts("cleanup network support\n");
        sharpen::CleanupNetSupport();
    });
    char ip[21];
    std::memset(ip,0,sizeof(ip));
    addr.GetAddrSring(ip,sizeof(ip));
    std::printf("now listen on %s:%d\n",ip,addr.GetPort());
    std::printf("use ctrl + c to stop\n");
    server.StartAsync();
    engine.Run();
}

struct A
{};

void AwaitTest()
{
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    engine.LaunchAndRun([]()
    {
        auto f1 = sharpen::Async([]()
        {
            std::printf("hello ");
            return 1;
        });
        auto f2 = sharpen::Async([](){
            std::printf("world\n");
        });
        auto f3 = sharpen::Async([](){
            return "hello world";
        });
        int r;
        std::tie(r,std::ignore,std::ignore) = sharpen::AwaitAll(*f1,*f2,*f3);
        std::printf("ret %d\n",r);
        auto f4 = sharpen::Async([]()
        {
            sharpen::Delay(std::chrono::seconds(9));
            std::printf("ok1\n");
        });
        auto f5 = sharpen::Async([](){
            sharpen::Delay(std::chrono::seconds(1));
            std::printf("ok2\n");
        });
        sharpen::AwaitAny(*f5,*f4);
        std::printf("done\n");
    });
}

void TimeWheelTest()
{
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    engine.LaunchAndRun([]()
    {
        sharpen::TimerPtr timer = sharpen::MakeTimer(sharpen::EventEngine::GetEngine());
        sharpen::TimeWheel wheel(std::chrono::seconds(1),60,timer);
        sharpen::TimeWheelPtr upstream = std::make_shared<sharpen::TimeWheel>(std::chrono::minutes(1),60);
        upstream->Put(std::chrono::minutes(1),[&wheel]() mutable
        {
            std::printf("ok\n");
            wheel.Stop();
        });
        wheel.SetUpstream(upstream);
        wheel.Put(std::chrono::seconds(10),[&wheel]() mutable
        {
            std::printf("hello world\n");
        });
        wheel.RunAsync();
    });
}

void ResetTest()
{
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    engine.LaunchAndRun([]()
    {
        sharpen::AwaitableFuture<void> future;
        sharpen::Launch([&future](){
            future.Complete();
        });
        future.Await();
        future.Reset();
        sharpen::Launch([&future](){
            future.Complete();
        });
        future.Await();
        std::printf("done\n");
    });
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
    return 0;
}