#include <cstdio>
#include <cassert>
#include <iterator>
#include <sharpen/Dns.hpp>
#include <sharpen/EventEngine.hpp>
#include <sharpen/INetStreamChannel.hpp>
#include <sharpen/IpEndPoint.hpp>
#include <sharpen/Ipv6EndPoint.hpp>

void Entry()
{
    sharpen::StartupNetSupport();
    std::puts("dns test begin1");
    std::vector<sharpen::Dns::ResolveResult> result;
    sharpen::Dns::ResolveName("localhost",std::back_inserter(result));
    for (auto begin = result.begin(),end = result.end(); begin != end; ++begin)
    {
        if(begin->af_ == sharpen::AddressFamily::Ip)
        {
            sharpen::IpEndPoint *ep = dynamic_cast<sharpen::IpEndPoint*>(begin->endPoint_.get());
            char buf[25] = {0};
            ep->GetAddrString(buf,sizeof(buf));
            std::printf("Ipv4\t%s\n",buf);
        }   
        else
        {
            sharpen::Ipv6EndPoint *ep = dynamic_cast<sharpen::Ipv6EndPoint*>(begin->endPoint_.get());
            char buf[60] = {0};
            ep->GetAddrString(buf,sizeof(buf));
            std::printf("Ipv4\t%s\n",buf);
        }
    }
    //assert(!result.empty());
    std::puts("pass");
    sharpen::CleanupNetSupport();
}

int main(int argc, char const *argv[])
{
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    engine.Startup(&Entry);
    return 0;
}
