#include <cstdio>
#include <string>
#include <sharpen/IFileChannel.hpp>
#include <sharpen/EventEngine.hpp>
#include <sharpen/INetStreamChannel.hpp>
#include <sharpen/IpEndPoint.hpp>
#include <sharpen/AsyncOps.hpp>
#include <sharpen/CtrlHandler.hpp>
#include <cstring>
#include <sharpen/ProcessInfo.hpp>
#include <sharpen/HttpServer.hpp>
#include <sharpen/TimeWheel.hpp>
#include <sharpen/AwaitOps.hpp>
#include <sharpen/Console.hpp>
#include <sharpen/TypeTraits.hpp>
#include <sharpen/StopWatcher.hpp>
#include <iostream>
#include <sharpen/AsyncMutex.hpp>

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
    char ip[21];
    std::memset(ip,0,sizeof(ip));
    addr.GetAddrSring(ip,sizeof(ip));
    std::printf("now listen on %s:%d\n",ip,addr.GetPort());
    std::printf("use ctrl + c to stop\n");
    sharpen::RegisterCtrlHandler(sharpen::CtrlType::Interrupt,[]()
    {
        std::puts("stop now\n");
        sharpen::EventEngine::GetEngine().Stop();
        std::puts("cleanup network support\n");
        sharpen::CleanupNetSupport();
    });
    engine.LaunchAndRun([&server]()
    {
        server.RunAsync();
    });
}

void AwaitTest()
{
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    engine.Startup([]()
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
    engine.Startup([]()
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
    engine.Startup([]()
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

void ConsoleTest()
{
    //print
    //output c style string
    sharpen::Print("hello world\n");
    //printf
    sharpen::Print("the number is ",1,"\n","float is ",1.00,"\n");
    //output ptr
    sharpen::Print("null pointer is ",nullptr,"\n");
    //output bool
    sharpen::Print("token is ",true,"\n");
    //%x & %X
    sharpen::Print("255 dec is ",sharpen::DecFormat<int>(255),"\n");
    sharpen::Print("255 hex is ",sharpen::HexFormat<int>(255),"\n");
    //0b
    sharpen::Print("255 bin is ",sharpen::BinFormat<int>(255),"\n");
    //output std::string
    std::string str{"std::string\n"};
    sharpen::Print(str);
}

template<typename _T>
using HasFunc = auto(*)()->decltype(std::declval<_T>().Func());

struct A
{
    void Func();
};

void ValidTest()
{
    bool tmp = sharpen::IsMatches<HasFunc,A>::Value;
    sharpen::Print("a has func? ",tmp,"\n");
    tmp = sharpen::IsMatches<HasFunc,int>::Value;
    sharpen::Print("int has func? ",tmp,"\n");
}

bool IsPrimeEx(size_t num)
{
	if (num <= 3) {
		return num > 1;
	}

	// If a number is not between 6 it can not be a prime number
	if (num % 6 != 1 && num % 6 != 5) {
		return false;
	}
	size_t tmp = static_cast<size_t>(sqrt(static_cast<double>(num)));
	for (size_t i = 5; i <= tmp; i += 6) {
		if (num % i == 0 || num % (i + 2) == 0) {
			return false;
		}
	}
	return true;
}

void ParallelTest(size_t n)
{
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupEngine();
    engine.Startup([n]()
    {
        sharpen::StopWatcher sw;
        sw.Begin();
        sharpen::ParallelFor(0,n,[](size_t i)
        {
            if (IsPrimeEx(i))
            {
                sharpen::Print(i,"\t");
            }
        });
        sw.Stop();
        double pt = sw.Compute()/CLOCKS_PER_SEC;
        sw.Begin();
        for (size_t i = 0; i < n; i++)
        {
            if (IsPrimeEx(i))
            {
                sharpen::Print(i,"\t");
            }
        }
        sw.Stop();
        double nt = sw.Compute()/CLOCKS_PER_SEC;
        sharpen::Print("\nparallel version using ",pt," sec\n","normal version using ",nt," sec\n");
        std::vector<size_t> data;
        for (size_t i = 0; i < data.size(); i++)
        {
            data.push_back(i);
        }
        sw.Begin();
        std::function<void(decltype(data)::iterator)> fn = [](decltype(data)::iterator ite)
        {
            sharpen::Print(*ite,"\t");
        };
        sharpen::ParallelForeach(data.begin(),data.end(),fn);
        sw.Stop();
        pt = sw.Compute();
        sw.Begin();
        for (auto &&i : data)
        {
            sharpen::Print(i,"\t");
        }
        sw.Stop();
        nt = sw.Compute();
        sharpen::Print("\nparallel version using ",pt," sec\n","normal version using ",nt," sec\n");
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