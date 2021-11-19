#include <cstdio>

#include <sharpen/MicroRpcField.hpp>
#include <sharpen/HttpRequest.hpp>
#include <sharpen/HttpRequestEncoder.hpp>
#include <sharpen/StopWatcher.hpp>
#include <sharpen/MicroRpcStack.hpp>
#include <sharpen/MicroRpcEncoder.hpp>

#define TEST_COUNT static_cast<sharpen::Size>(1e6)
#define TEST_ARG 4

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
            sharpen::HttpRequest req(sharpen::HttpMethod::GET, "/Hello", sharpen::HttpVersion::Http1_1);
            for (size_t j = 0; j < TEST_ARG; ++j)
            {
                int ran = rand();
                char buf[1024] = {0};
                snprintf(buf, 1024, "{%zu:%d}\n",j,ran);
                req.Body().Append(buf, strlen(buf));
            }
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
                stack.Push(rand());
            }
            sharpen::ByteBuffer &&buf = sharpen::MicroRpcEncoder::Encode(stack);
            sum += buf.GetSize();
        }
        sw.Stop();
        std::printf("micro rpc with %d arg using %d tu using %zu bytes\n",TEST_ARG,sw.Compute(), sum);
    }
    return 0;
}
