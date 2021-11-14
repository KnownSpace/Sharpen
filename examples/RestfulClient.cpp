#include <sharpen/RpcClient.hpp>
#include <sharpen/HttpRequest.hpp>
#include <sharpen/HttpResponse.hpp>
#include <sharpen/HttpRequestDecoder.hpp>
#include <sharpen/HttpResponseDecoder.hpp>
#include <sharpen/HttpRequestEncoder.hpp>
#include <sharpen/HttpResponseEncoder.hpp>
#include <sharpen/IpEndPoint.hpp>


using HttpClient = sharpen::RpcClient<sharpen::HttpRequest,sharpen::HttpRequestEncoder,sharpen::HttpResponse,sharpen::HttpResponseDecoder>;

void Entry()
{
    sharpen::HttpResponseDecoder decoer;
    sharpen::StartupNetSupport();
    sharpen::IpEndPoint addr;
    addr.SetAddrByString("127.0.0.1");
    addr.SetPort(0);
    sharpen::NetStreamChannelPtr conn = sharpen::MakeTcpStreamChannel(sharpen::AddressFamily::Ip);
    conn->Bind(addr);
    conn->Register(sharpen::EventEngine::GetEngine());
    addr.SetPort(8080);
    conn->ConnectAsync(addr);
    HttpClient client(conn);
    sharpen::HttpRequest req(sharpen::HttpMethod::GET,"/",sharpen::HttpVersion::Http1_1);
    sharpen::HttpResponse &&res = client.InvokeAsync(req);
    std::printf("Status code: %s\n",sharpen::GetHttpStatusCodeName(res.StatusCode()));
    std::printf("Headers:\n");
    for (auto begin = res.Header().Begin(),end = res.Header().End();begin != end;begin++)
    {
        std::printf("%s:%s\n",begin->first.c_str(),begin->second.c_str());
    }
    std::printf("Body:\n");
    for (sharpen::Size i = 0; i < res.Body().GetSize(); i++)
    {
        std::putchar(res.Body()[i]);
    }
    sharpen::CleanupNetSupport();
}

int main(int argc, char const *argv[])
{
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupEngine();
    engine.Startup(&Entry);
    return 0;
}
