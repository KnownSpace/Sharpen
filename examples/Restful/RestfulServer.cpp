#include <sharpen/RpcServer.hpp>
#include <sharpen/HttpRequest.hpp>
#include <sharpen/HttpResponse.hpp>
#include <sharpen/HttpRequestDecoder.hpp>
#include <sharpen/HttpResponseEncoder.hpp>
#include <sharpen/IpEndPoint.hpp>
#include <sharpen/CtrlHandler.hpp>

struct RestfulDispatcher
{
    static const std::string &GetProcedureName(const sharpen::HttpRequest &req)
    {
        return req.Url();
    }
};

using RestfulServer = sharpen::RpcServer<sharpen::HttpResponse,sharpen::HttpResponseEncoder,sharpen::HttpRequest,sharpen::HttpRequestDecoder,RestfulDispatcher>;

using Option = typename RestfulServer::Option;

using Context = typename RestfulServer::Context;

void Entry()
{
    sharpen::StartupNetSupport();
    Option opt(RestfulDispatcher{},std::chrono::seconds(3));
    sharpen::IpEndPoint addr;
    addr.SetAddrByString("127.0.0.1");
    addr.SetPort(8081);
    RestfulServer server(sharpen::AddressFamily::Ip,addr,sharpen::EventEngine::GetEngine(),std::move(opt));
    server.Register("/Hello",[](Context &ctx)
    {
        std::printf("call from remote\n");
        sharpen::HttpResponse res(sharpen::HttpVersion::Http1_1,sharpen::HttpStatusCode::OK);
        const char content[] = "Hello world";
        res.Header()["Content-Type"] = "text/plain";
        res.Header()["Content-Length"] = std::to_string(sizeof(content) - 1);
        res.Body().CopyFrom(content,sizeof(content) - 1);
        sharpen::ByteBuffer &&buf = ctx.Encoder().Encode(res);
        ctx.Connection()->WriteAsync(buf);
    });
    server.Register("/Benchmark",[](Context &ctx)
    {
        sharpen::HttpResponse res(sharpen::HttpVersion::Http1_1,sharpen::HttpStatusCode::OK);
        const char content[] = "Hello world";
        res.Header()["Content-Type"] = "text/plain";
        res.Header()["Content-Length"] = std::to_string(sizeof(content) - 1);
        res.Body().CopyFrom(content,sizeof(content) - 1);
        thread_local static sharpen::ByteBuffer buf{4096};
        sharpen::Size size = ctx.Encoder().EncodeTo(res,buf);
        ctx.Connection()->WriteAsync(buf.Data(),size);
    });
    server.RegisterTimeout([](Context &ctx)
    {
        sharpen::IpEndPoint addr;
        ctx.Connection()->GetRemoteEndPoint(addr);
        char ip[21] = {0};
        addr.GetAddrString(ip,sizeof(ip));
        std::printf("%s:%u timeout disconnect\n",ip,addr.GetPort());
    });
    sharpen::RegisterCtrlHandler(sharpen::CtrlType::Interrupt,[&server]() mutable
    {
        std::printf("stop now\n");
        server.Stop();
    });
    server.RunAsync();
    sharpen::CleanupNetSupport();
}

int main(int argc, char const *argv[])
{
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupEngine();
    engine.Startup(&Entry);
    return 0;
}
