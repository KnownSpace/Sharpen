#include <cassert>
#include <cstdio>
#include <cstring>

#include <sharpen/IFileChannel.hpp>
#include <sharpen/EventEngine.hpp>
#include <sharpen/FileOps.hpp>

void Test()
{
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    std::printf("file test begin\n");
    sharpen::FileChannelPtr channel = sharpen::MakeFileChannel("./hello.txt", sharpen::FileAccessModel::Write, sharpen::FileOpenModel::CreateOrOpen);
    channel->Register(engine);
    char str[] = "hello";
    sharpen::Size size = channel->WriteAsync(str, sizeof(str) - 1, 0);
    std::printf("write size is %zu\n", size);
    assert(size == sizeof(str) - 1);
    std::printf("pass\n");
    char buf[sizeof(str)] = {0};
    channel->Close();
    channel = sharpen::MakeFileChannel("./hello.txt", sharpen::FileAccessModel::Read, sharpen::FileOpenModel::Open);
    channel->Register(engine);
    size = channel->ReadAsync(buf, sizeof(buf) - 1, 0);
    std::printf("read size is %zu\n", size);
    for (size_t i = 0; i < sizeof(str) - 1; i++)
    {
        assert(buf[i] == str[i]);
    }
    std::printf("pass\n");
    std::printf("zero memory test\n");
    channel = sharpen::MakeFileChannel("./buf.log", sharpen::FileAccessModel::Write, sharpen::FileOpenModel::CreateNew);
    channel->Register(engine);
    channel->ZeroMemoryAsync(64 * 1024, 0);
    assert(channel->GetFileSize() == 64 * 1024);
    channel->Close();
    std::printf("pass\n");
    std::printf("exist test\n");
    assert(sharpen::ExistFile("./buf.log"));
    std::printf("pass\n");
    std::printf("access test\n");
    assert(sharpen::AccessFile("./buf.log", sharpen::FileAccessModel::Read));
    std::printf("pass\n");
    std::printf("rename test\n");
    sharpen::RenameFile("./buf.log", "./buf1.log");
    assert(!sharpen::ExistFile("./buf.log"));
    assert(sharpen::ExistFile("./buf1.log"));
    std::printf("pass\n");
    std::printf("remove test\n");
    sharpen::RemoveFile("./buf1.log");
    assert(!sharpen::ExistFile("./buf1.log"));
    std::printf("pass\n");
    std::printf("map file test\n");
    channel = sharpen::MakeFileChannel("./buf.log", sharpen::FileAccessModel::All, sharpen::FileOpenModel::CreateNew);
    channel->Register(engine);
    channel->ZeroMemoryAsync(64*1024, 0);
    {
        auto mem = channel->MapMemory(12, 0);
        std::memcpy(mem.Get(), "Hello World", 11);
        mem.Flush();
        channel->Close();
    }
    sharpen::RemoveFile("./buf.log");
    sharpen::RemoveFile("./hello.txt");
    std::printf("pass\n");
    std::printf("file test pass\n");
}

void FileTest()
{
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    engine.Startup([&engine]() {
        try
        {
            Test();
        }
        catch(const std::exception& e)
        {
            std::printf("error: %s\n",e.what());   
        }
    });
}

int main()
{
    FileTest();
    return 0;
}
