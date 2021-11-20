#include <sharpen/HttpRequest.hpp>
#include <sharpen/HttpRequestDecoder.hpp>
#include <sharpen/HttpRequestEncoder.hpp>
#include <sharpen/MicroRpcStack.hpp>
#include <sharpen/MicroRpcEncoder.hpp>
#include <sharpen/MicroRpcDecoder.hpp>
#include <sharpen/StopWatcher.hpp>
#include <sharpen/Converter.hpp>

#define TEST_COUNT static_cast<sharpen::Size>(1e7)
#define TEST_ARGS 3

int main(int argc, char const *argv[])
{
    sharpen::StopWatcher sw;
    sharpen::ByteBuffer buf;
    buf.Reserve(4096);
    //http
    {
        sharpen::HttpRequest req(sharpen::HttpMethod::GET,"/Hello",sharpen::HttpVersion::Http1_1);
        req.Header()["Connection"] = "Keep-alive";
        req.CopyTo(buf);
    }
    {
        sw.Begin();
        sharpen::Size sum{0};
        for (size_t i = 0; i < TEST_COUNT; ++i)
        {
            sharpen::HttpRequest req;
            sharpen::HttpRequestDecoder decoder;
            decoder.Bind(req);
            sharpen::Size size = decoder.Decode(buf.Data(),buf.GetSize());
            assert(size == buf.GetSize());
            sum += size;
        }
        sw.Stop();
        std::printf("http using %d tu with %zu bytes\n",sw.Compute(),sum);
    }
    buf.Clear();
    //micro rpc
    {
        sharpen::MicroRpcStack stack;
        char proc[] = "Hello";
        stack.Push(proc,proc + sizeof(proc) - 1);
        stack.CopyTo(buf);
    }
    {
        sw.Begin();
        sharpen::Size sum{0};
        for (size_t i = 0; i < TEST_COUNT; i++)
        {
            sharpen::MicroRpcStack stack;
            sharpen::MicroRpcDecoder decoder;
            decoder.Bind(stack);
            sharpen::Size size = decoder.Decode(buf.Data(),buf.GetSize());
            assert(size == buf.GetSize());
            sum += size;
            decoder.Decode(buf.Data(),buf.GetSize());
        }
        sw.Stop();
        std::printf("micro rpc using %d tu with %zu bytes\n",sw.Compute(),sum);
    }
    buf.Clear();
    srand(0);
    //http arg
    {
        sharpen::HttpRequest req(sharpen::HttpMethod::GET,"/Hello",sharpen::HttpVersion::Http1_1);
        req.Header()["Connection"] = "Keep-alive";
        sharpen::Size sum{0};
        for (size_t i = 0; i < TEST_ARGS; i++)
        {
            char body[1024] = {0};
            std::snprintf(body,1024,"%d\n",std::rand());
            sharpen::Size size = std::strlen(body);
            req.Body().Append(body,size);
            sum += size;
        }
        req.Header()["Content-Length"] = std::to_string(sum);
        req.CopyTo(buf);
    }
    {
        sharpen::Size sum{0};
        sw.Begin();
        int args[TEST_ARGS] = {0};
        for (size_t i = 0; i < TEST_COUNT; ++i)
        {
            sharpen::HttpRequest req;
            sharpen::HttpRequestDecoder decoder;
            decoder.Bind(req);
            sharpen::Size size = decoder.Decode(buf.Data(),buf.GetSize());
            assert(size == buf.GetSize());
            sum += size;
            sharpen::Size j{0};
            for(auto begin = req.Body().Begin(),end = req.Body().End(),pred = begin;begin != end;++begin)
            {
                if(*begin == '\n')
                {
                    args[j++] += sharpen::Atoi<int>(&*pred,begin - pred);
                    pred = begin + 1;
                }
            }
        }
        sw.Stop();
        std::printf("http with %d args using %d tu with %zu bytes\n",TEST_ARGS,sw.Compute(),sum);
        std::printf("arg is:");
        for(size_t i = 0;i < TEST_ARGS;++i)
        {
            std::printf("%d ",args[i]);
        }
        std::printf("\n");
    }
    //micro rpc arg
    buf.Clear();
    std::srand(0);
    {
        sharpen::MicroRpcStack stack;
        char proc[] = "Hello";
        for (size_t i = 0; i < TEST_ARGS; i++)
        {
            stack.Push(std::rand());
        }
        stack.Reverse();
        stack.Push(proc,proc + sizeof(proc) - 1);
        stack.CopyTo(buf);
    }
    {
        sw.Begin();
        sharpen::Size sum{0};
        int args[TEST_ARGS] = {0};
        for (size_t i = 0; i < TEST_COUNT; i++)
        {
            sharpen::MicroRpcStack stack;
            sharpen::MicroRpcDecoder decoder;
            decoder.Bind(stack);
            sharpen::Size size = decoder.Decode(buf.Data(),buf.GetSize());
            assert(size == buf.GetSize());
            sum += size;
            sharpen::Size index{0};
            for (auto begin = ++stack.Begin(),end = stack.End();begin != end;++begin)
            {
                args[index++] += *begin->Data<int>();
            }
        }
        sw.Stop();
        std::printf("micro rpc with %d args using %d tu with %zu bytes\n",TEST_ARGS,sw.Compute(),sum);
        std::printf("arg is:");
        for(size_t i = 0;i < TEST_ARGS;++i)
        {
            std::printf("%d ",args[i]);
        }
        std::printf("\n");
    }
    return 0;
}
