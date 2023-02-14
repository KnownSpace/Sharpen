#include <cassert>
#include <cstdio>
#include <cstring>

#include <sharpen/IFileChannel.hpp>
#include <sharpen/EventEngine.hpp>
#include <sharpen/FileOps.hpp>
#include <sharpen/AlignedAlloc.hpp>
#include <sharpen/MemoryPage.hpp>

#include <simpletest/TestRunner.hpp>

class WriteTest :public simpletest::ITypenamedTest<WriteTest>
{
private:
    using Self = WriteTest;

public:

    WriteTest() noexcept = default;

    ~WriteTest() noexcept = default;

    inline const Self &Const() const noexcept
    {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept
    {
        sharpen::FileChannelPtr channel = sharpen::OpenFileChannel("./hello.txt",sharpen::FileAccessMethod::Write,sharpen::FileOpenMethod::CreateOrOpen);
        channel->Register(sharpen::GetLocalLoopGroup());
        char str[] = "hello";
        std::size_t size = channel->WriteAsync(str,sizeof(str) - 1,0);
        channel->FlushAsync();
        return this->Assert(size == sizeof(str) - 1,"size should == sizeof(str) - 1,but it not");
    }
};

class ReadTest :public simpletest::ITypenamedTest<ReadTest>
{
private:
    using Self = ReadTest;

public:

    ReadTest() noexcept = default;

    ~ReadTest() noexcept = default;

    inline const Self &Const() const noexcept
    {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept
    {
        sharpen::FileChannelPtr channel = sharpen::OpenFileChannel("./hello.txt",sharpen::FileAccessMethod::Read,sharpen::FileOpenMethod::CreateOrOpen);
        channel->Register(sharpen::GetLocalLoopGroup());
        char buf[6] = {0};
        std::size_t size = channel->ReadAsync(buf,sizeof(buf) - 1,0);
        channel->Close();
        sharpen::RemoveFile("./hello.txt");
        return this->Assert(!std::strncmp(buf,"hello",(std::min)(sizeof(buf) - 1,size)),"buf should == \"hello\",but it not");
    }
};

class ZeroMemoryTest:public simpletest::ITypenamedTest<ZeroMemoryTest>
{
private:
    using Self = ZeroMemoryTest;

public:

    ZeroMemoryTest() noexcept = default;

    ~ZeroMemoryTest() noexcept = default;

    inline const Self &Const() const noexcept
    {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept
    {
        sharpen::FileChannelPtr channel = sharpen::OpenFileChannel("./buf.log",sharpen::FileAccessMethod::Write,sharpen::FileOpenMethod::CreateNew);
        channel->Register(sharpen::GetLocalLoopGroup());
        channel->ZeroMemoryAsync(64 * 1024,0);
        return this->Assert(channel->GetFileSize() == 64 * 1024,"File size should == 6 * 1024,but it not");
    }
};

class ExistTest:public simpletest::ITypenamedTest<ExistTest>
{
private:
    using Self = ExistTest;

public:

    ExistTest() noexcept = default;

    ~ExistTest() noexcept = default;

    inline const Self &Const() const noexcept
    {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept
    {
        return this->Assert(sharpen::ExistFile("./buf.log"),"ExistFile() return wrong answer");
    }
};

class AccessTest:public simpletest::ITypenamedTest<AccessTest>
{
private:
    using Self = AccessTest;

public:

    AccessTest() noexcept = default;

    ~AccessTest() noexcept = default;

    inline const Self &Const() const noexcept
    {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept
    {
        return this->Assert(sharpen::AccessFile("./buf.log",sharpen::FileAccessMethod::Read),"AccessFile() return wrong answer");
    }
};

class RenameTest:public simpletest::ITypenamedTest<RenameTest>
{
private:
    using Self = RenameTest;

public:

    RenameTest() noexcept = default;

    ~RenameTest() noexcept = default;

    inline const Self &Const() const noexcept
    {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept
    {
        sharpen::RenameFile("./buf.log","./buf1.log");
        return this->Assert(!sharpen::ExistFile("./buf.log") && sharpen::ExistFile("./buf1.log"),"RenameFile() doesn't works");
    }
};

class RemoveTest:public simpletest::ITypenamedTest<RemoveTest>
{
private:
    using Self = RemoveTest;

public:

    RemoveTest() noexcept = default;

    ~RemoveTest() noexcept = default;

    inline const Self &Const() const noexcept
    {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept
    {
        sharpen::RemoveFile("./buf1.log");
        return this->Assert(!sharpen::ExistFile("./buf1.log"),"RemoveFile() doesn't works");
    }
};

class MappingTest:public simpletest::ITypenamedTest<MappingTest>
{
private:
    using Self = MappingTest;

public:

    MappingTest() noexcept = default;

    ~MappingTest() noexcept = default;

    inline const Self &Const() const noexcept
    {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept
    {
        sharpen::FileChannelPtr channel = sharpen::OpenFileChannel("./buf.log",sharpen::FileAccessMethod::All,sharpen::FileOpenMethod::CreateNew);
        channel->Register(sharpen::GetLocalLoopGroup());
        channel->ZeroMemoryAsync(64*1024,0);
        char data[] = "Hello World";
        {
            auto mem = channel->MapMemory(12,0);
            std::memcpy(mem.Get(),data,sizeof(data) - 1);
            mem.Flush();
        }
        {
            auto mem = channel->MapMemory(12,0);
            return this->Assert(!std::strncmp(data,reinterpret_cast<char*>(mem.Get()),sizeof(data) - 1),"Mapping memory should == \"Hello World\",but it not");
        }
    }
};

class ResolvePathTest:public simpletest::ITypenamedTest<ResolvePathTest>
{
private:
    using Self = ResolvePathTest;

public:

    ResolvePathTest() noexcept = default;

    ~ResolvePathTest() noexcept = default;

    inline const Self &Const() const noexcept
    {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept
    {
        bool status{true};
        {
            char curr[] = "/";
            char path[] = "./abc/def/.././a.txt/.a";
            char resolved[sizeof(curr) + sizeof(path) - 1] = {0};
            // /abc/a.txt/.a
            sharpen::ResolvePath(curr,sizeof(curr) - 1,path,sizeof(path) - 1,resolved,sizeof(resolved) - 1);
            status = status && !std::strcmp(resolved,"/abc/a.txt/.a");
        }
        {
            char curr[] = "C:/";
            char path[] = "./abc/def/.././a.txt/.a";
            char resolved[sizeof(curr) + sizeof(path) - 1] = {0};
            // C:/abc/a.txt/.a
            sharpen::ResolvePath(curr,sizeof(curr) - 1,path,sizeof(path) - 1,resolved,sizeof(resolved) - 1);
            status = status && !std::strcmp(resolved,"C:/abc/a.txt/.a");
        }
        {
            char path[] = "./abc/def/.././a.txt/.a";
            char resolved[sizeof(path)] = {0};
            // abc/a.txt/.a
            sharpen::ResolvePath(nullptr,0,path,sizeof(path) - 1,resolved,sizeof(resolved) - 1);
            status = status && !std::strcmp(resolved,"abc/a.txt/.a");
        }
        return this->Assert(status,"ResolvePath() return wrong answer");
    }
};

class DirectOpeartionTest:public simpletest::ITypenamedTest<DirectOpeartionTest>
{
private:
    using Self = DirectOpeartionTest;

public:

    DirectOpeartionTest() noexcept = default;

    ~DirectOpeartionTest() noexcept = default;

    inline const Self &Const() const noexcept
    {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept
    {
        sharpen::FileChannelPtr channel = sharpen::OpenFileChannel("./buf.log",sharpen::FileAccessMethod::All,sharpen::FileOpenMethod::CreateNew,sharpen::FileIoMethod::DirectAndSync);
        channel->Register(sharpen::GetLocalLoopGroup());
        sharpen::MemoryPage content{1};
        std::memcpy(content.Data(),"1234",4);
        channel->WriteAsync(content.Data(),content.GetSize(),0);
        std::memset(content.Data(),0,content.GetSize());
        channel->ReadAsync(content.Data(),content.GetSize(),0);
        channel->Close();
        sharpen::RemoveFile("./buf.log");
        return this->Assert(!std::strncmp(content.Data(),"1234",4),"content should == \"1234\",but it not");
    }
};

static int Test()
{
    simpletest::TestRunner runner;
    runner.Register<WriteTest>();
    runner.Register<ReadTest>();
    runner.Register<ZeroMemoryTest>();
    runner.Register<ExistTest>();
    runner.Register<AccessTest>();
    runner.Register<RenameTest>();
    runner.Register<RemoveTest>();
    runner.Register<MappingTest>();
    runner.Register<ResolvePathTest>();
    runner.Register<DirectOpeartionTest>();
    return runner.Run();
}

int main()
{
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    return engine.StartupWithCode(&Test);
}
