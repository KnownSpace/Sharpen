#include <cstdio>
#include <cassert>
#include <iterator>
#include <sharpen/Dns.hpp>
#include <sharpen/EventEngine.hpp>
#include <sharpen/INetStreamChannel.hpp>
#include <sharpen/IpEndPoint.hpp>
#include <sharpen/Ipv6EndPoint.hpp>

void Entry(const char *name)
{
    sharpen::StartupNetSupport();
    std::vector<sharpen::Dns::ResolveResult> result;
    sharpen::Dns::ResolveName(name,std::back_inserter(result));
    if(result.empty())
    {
        std::puts("host not found");
        return;
    }
    std::puts("all dns results:");
    auto begin = result.begin(),end = result.end();
    if(!begin->canonname_.Empty())
    {
        std::fputs("Canonname\t",stdout);
        for (auto ite = begin->canonname_.Begin(); ite != begin->canonname_.End(); ++ite)
        {
            std::putchar(*ite);
        }
        std::putchar('\n');
    }
    for (; begin != end; ++begin)
    {
        if(begin->af_ == sharpen::AddressFamily::Ip)
        {
            sharpen::IpEndPoint *ep = dynamic_cast<sharpen::IpEndPoint*>(begin->endPoint_.get());
            char buf[25] = {0};
            ep->GetAddrString(buf,sizeof(buf));
            std::printf("Ipv4     \t%s\n",buf);
        }   
        else
        {
            sharpen::Ipv6EndPoint *ep = dynamic_cast<sharpen::Ipv6EndPoint*>(begin->endPoint_.get());
            char buf[60] = {0};
            ep->GetAddrString(buf,sizeof(buf));
            std::printf("Ipv6     \t%s\n",buf);
        }
    }
    sharpen::CleanupNetSupport();
}

void PrintUsage()
{
    std::puts("usage: <hostname> - list dns results");
}

int main(int argc, char const *argv[])
{
    if(argc < 2)
    {
        PrintUsage();
        return 0;
    }
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    engine.Startup(&Entry,argv[1]);
    return 0;
}