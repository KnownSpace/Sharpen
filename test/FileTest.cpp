#include <cassert>
#include <cstdio>

#include <sharpen/IFileChannel.hpp>
#include <sharpen/EventEngine.hpp>

void FileTest()
{
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    engine.Startup([&engine]()
    {
        std::printf("file test begin\n");
        sharpen::FileChannelPtr channel = sharpen::MakeFileChannel("./hello.txt",sharpen::FileAccessModel::Write,sharpen::FileOpenModel::CreateOrOpen);
        channel->Register(engine);
        char str[] = "hello";
        sharpen::Size size = channel->WriteAsync(str,sizeof(str) - 1,0);
        std::printf("write test pass\n");
        assert(size == sizeof(str)-1);
        char buf[4096] = {0};
        channel->Close();
        channel = sharpen::MakeFileChannel("./hello.txt",sharpen::FileAccessModel::Read,sharpen::FileOpenModel::Open);
        channel->Register(engine);
        size = channel->ReadAsync(buf,sizeof(buf),0);
        assert(size == sizeof(str)-1);
        for (size_t i = 0; i < sizeof(str) -1; i++)
        {
            assert(buf[i] == str[i]);
        }
        std::printf("read test pass\n");
        std::printf("file test pass\n");
    });
}

int main()
{
    FileTest();
    return 0;
}
