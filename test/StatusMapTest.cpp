#include <sharpen/IStatusMap.hpp>
#include <sharpen/SimpleStatusMap.hpp>
#include <sharpen/EventEngine.hpp>
#include <sharpen/FileOps.hpp>

#include <simpletest/TestRunner.hpp>

class MapTest:public simpletest::ITypenamedTest<MapTest>
{
private:
    using Self = MapTest;
public:

    MapTest() noexcept = default;

    ~MapTest() noexcept = default;

    inline const Self &Const() const noexcept
    {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept
    {
        sharpen::FileChannelPtr channel = sharpen::OpenFileChannel("./Test.bin",sharpen::FileAccessMethod::All,sharpen::FileOpenMethod::CreateNew);
        channel->Register(sharpen::GetLocalLoopGroup());
        std::unique_ptr<sharpen::IStatusMap> map{new (std::nothrow) sharpen::SimpleStatusMap{std::move(channel)}};
        sharpen::ByteBuffer key{"key",3};
        sharpen::ByteBuffer value{"val",3};
        map->Write(key,value);
        auto valOpt = map->Lookup(key);
        return this->Assert(valOpt.Exist() && valOpt.Get() == value,"Map test return wrong answer");
    }
};

class PersistentTest:public simpletest::ITypenamedTest<PersistentTest>
{
private:
    using Self = PersistentTest;
public:

    PersistentTest() noexcept = default;

    ~PersistentTest() noexcept = default;

    inline const Self &Const() const noexcept
    {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept
    {
        sharpen::FileChannelPtr channel = sharpen::OpenFileChannel("./Test.bin",sharpen::FileAccessMethod::All,sharpen::FileOpenMethod::Open);
        channel->Register(sharpen::GetLocalLoopGroup());
        std::unique_ptr<sharpen::IStatusMap> map{new (std::nothrow) sharpen::SimpleStatusMap{std::move(channel)}};
        sharpen::ByteBuffer key{"key",3};
        sharpen::ByteBuffer value{"val",3};
        auto valOpt = map->Lookup(key);
        return this->Assert(valOpt.Exist() && valOpt.Get() == value,"Persistent test return wrong answer");
    }
};

static int Test()
{
    simpletest::TestRunner runner;
    runner.Register<MapTest>();
    runner.Register<PersistentTest>();
    int code{runner.Run()};
    sharpen::RemoveFile("./Test.bin");
    return code;
}

int main(int argc, char const *argv[])
{
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();   
    return engine.StartupWithCode(&Test);
}