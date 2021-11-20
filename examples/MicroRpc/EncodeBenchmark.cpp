#include <cstdio>

#include <sharpen/MicroRpcField.hpp>
#include <sharpen/HttpRequest.hpp>
#include <sharpen/HttpRequestEncoder.hpp>
#include <sharpen/StopWatcher.hpp>
#include <sharpen/MicroRpcStack.hpp>
#include <sharpen/MicroRpcEncoder.hpp>

#define TEST_COUNT static_cast<sharpen::Size>(1e6)
#define TEST_ARG 3

int main(int argc, char const *argv[])
{
    sharpen::StopWatcher sw;
    //http
    {
        sw.Begin();
        sharpen::Size sum{0};
        for (size_t i = 0; i < TEST_COUNT; i++)
        {
            sharpen::HttpRequest req(sharpen::HttpMethod::GET, "/Hello", sharpen::HttpVersion::Http1_1);
            req.Header()["Connection"] = "keep-alive";
            sharpen::ByteBuffer &&buf = sharpen::HttpRequestEncoder::Encode(req);
            sum += buf.GetSize();
        }
        sw.Stop();
        std::printf("http using %d tu using %zu bytes\n", sw.Compute(), sum);
    }
    //micro
    {
        sw.Begin();
        sharpen::Size sum{0};
        for (size_t i = 0; i < TEST_COUNT; i++)
        {
            char proc[] = "Hello";
            sharpen::MicroRpcStack stack;
            stack.Push(proc,proc + sizeof(proc) - 1);
            sharpen::ByteBuffer &&buf = sharpen::MicroRpcEncoder::Encode(stack);
            sum += buf.GetSize();
        }
        sw.Stop();
        std::printf("micro rpc using %d tu using %zu bytes\n", sw.Compute(), sum);
    }
    srand(0);
    //http arg
    {
        sw.Begin();
        sharpen::Size sum{0};
        for (size_t i = 0; i < TEST_COUNT; i++)
        {
            sharpen::HttpRequest req(sharpen::HttpMethod::POST, "/Hello", sharpen::HttpVersion::Http1_1);
            for (size_t j = 0; j < TEST_ARG; ++j)
            {
                char buf[1024] = {0};
                snprintf(buf, 1024, "%d\n",std::rand());
                req.Body().Append(buf, std::strlen(buf));
            }
            req.Header()["Connection"] = "keep-alive";
            req.Header()["Content-Length"] = std::to_string(req.Body().GetSize());
            sharpen::ByteBuffer &&buf = sharpen::HttpRequestEncoder::Encode(req);
            sum += buf.GetSize();
        }
        sw.Stop();
        std::printf("http with %d arg using %d tu using %zu bytes\n",TEST_ARG,sw.Compute(), sum);
    }
    srand(0);
    //micro arg
    {
        sw.Begin();
        sharpen::Size sum{0};
        for (size_t i = 0; i < TEST_COUNT; i++)
        {
            char proc[] = "Hello";
            sharpen::MicroRpcStack stack;
            stack.Push(proc,proc + sizeof(proc) - 1);
            for (size_t j = 0; j < TEST_ARG; j++)
            {
                stack.Push(std::rand());
            }
            sharpen::ByteBuffer &&buf = sharpen::MicroRpcEncoder::Encode(stack);
            sum += buf.GetSize();
        }
        sw.Stop();
        std::printf("micro rpc with %d arg using %d tu using %zu bytes\n",TEST_ARG,sw.Compute(), sum);
    }
    return 0;
}
