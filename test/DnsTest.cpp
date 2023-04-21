#include <cassert>
#include <cstdio>
#include <iterator>

#include <sharpen/Dns.hpp>
#include <sharpen/EventEngine.hpp>
#include <sharpen/INetStreamChannel.hpp>
#include <sharpen/IpEndPoint.hpp>
#include <sharpen/Ipv6EndPoint.hpp>

#include <simpletest/TestRunner.hpp>

class LookupLocalhostTest : public simpletest::ITypenamedTest<LookupLocalhostTest>
{
private:
    using Self = LookupLocalhostTest;

public:
    LookupLocalhostTest() noexcept = default;

    ~LookupLocalhostTest() noexcept = default;

    inline const Self &Const() const noexcept
    {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept
    {
        std::vector<sharpen::DnsResolveResult> results;
        sharpen::Dns::ResolveName("localhost", std::back_inserter(results));
        bool status{false};
        for (auto begin = results.begin(), end = results.end(); begin != end; ++begin)
        {
            if (begin->GetAddressFamily() == sharpen::AddressFamily::Ip)
            {
                sharpen::IpEndPoint *ep =
                    static_cast<sharpen::IpEndPoint *>(begin->EndPointPtr().get());
                char buf[25] = {0};
                ep->GetAddrString(buf, sizeof(buf));
                if (!std::strncmp(buf, "127.0.0.1", 9))
                {
                    status = true;
                }
            }
        }
        return this->Assert(status, "result set should contains \"127.0.0.1\",but it not");
    }
};

static int Test()
{
    sharpen::StartupNetSupport();
    simpletest::TestRunner runner;
    runner.Register<LookupLocalhostTest>();
    int code{runner.Run()};
    sharpen::CleanupNetSupport();
    return code;
}

int main(int argc, char const *argv[])
{
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    return engine.StartupWithCode(&Test);
}