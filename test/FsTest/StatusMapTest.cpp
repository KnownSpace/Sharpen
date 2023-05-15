#include <sharpen/CowStatusMap.hpp>
#include <sharpen/EventEngine.hpp>
#include <sharpen/FileOps.hpp>
#include <sharpen/IStatusMap.hpp>
#include <sharpen/WalStatusMap.hpp>

#include <simpletest/TestRunner.hpp>

static const char *cowName = "./Test.cow";

static const char *walName = "./Test.wal";

static constexpr std::size_t keyCount{100};

class CowMapTest : public simpletest::ITypenamedTest<CowMapTest> {
private:
    using Self = CowMapTest;

public:
    CowMapTest() noexcept = default;

    ~CowMapTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        std::unique_ptr<sharpen::IStatusMap> map{new (std::nothrow) sharpen::CowStatusMap{cowName}};
        sharpen::ByteBuffer key{"key", 4};
        sharpen::ByteBuffer value{"val", 4};
        for (std::size_t i = 0; i != keyCount; ++i) {
            key[3] = static_cast<char>(i);
            value[3] = static_cast<char>(i);
            map->Write(key, value);
        }
        for (std::size_t i = 0; i != keyCount; ++i) {
            key[3] = static_cast<char>(i);
            value[3] = static_cast<char>(i);
            auto valOpt = map->Lookup(key);
            if (valOpt.Get() != value) {
                return this->Fail("Get() return wrong answer,put failed or lookup failed");
            }
        }
        for (std::size_t i = 0; i != keyCount; ++i) {
            key[3] = static_cast<char>(i);
            map->Remove(key);
        }
        for (std::size_t i = 0; i != keyCount; ++i) {
            key[3] = static_cast<char>(i);
            auto valOpt = map->Lookup(key);
            if (valOpt.Exist()) {
                return this->Fail("Get() return wrong answer,remove failed");
            }
        }
        return this->Success();
    }
};

class CowPersistentTest : public simpletest::ITypenamedTest<CowPersistentTest> {
private:
    using Self = CowPersistentTest;

public:
    CowPersistentTest() noexcept = default;

    ~CowPersistentTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        std::unique_ptr<sharpen::IStatusMap> map{new (std::nothrow) sharpen::CowStatusMap{cowName}};
        sharpen::ByteBuffer key{"key", 3};
        sharpen::ByteBuffer value{"val", 3};
        map->Write(key, value);
        map.reset(nullptr);
        map.reset(new (std::nothrow) sharpen::CowStatusMap{cowName});
        auto valOpt = map->Lookup(key);
        return this->Assert(valOpt.Exist() && valOpt.Get() == value,
                            "Get() return wrong answer,persistence failed");
    }
};

class WalMapTest : public simpletest::ITypenamedTest<WalMapTest> {
private:
    using Self = WalMapTest;

public:
    WalMapTest() noexcept = default;

    ~WalMapTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        std::unique_ptr<sharpen::IStatusMap> map{new (std::nothrow) sharpen::WalStatusMap{walName}};
        sharpen::ByteBuffer key{"key", 4};
        sharpen::ByteBuffer value{"val", 4};
        for (std::size_t i = 0; i != keyCount; ++i) {
            key[3] = static_cast<char>(i);
            value[3] = static_cast<char>(i);
            map->Write(key, value);
        }
        for (std::size_t i = 0; i != keyCount; ++i) {
            key[3] = static_cast<char>(i);
            value[3] = static_cast<char>(i);
            auto valOpt = map->Lookup(key);
            if (valOpt.Get() != value) {
                return this->Fail("Get() return wrong answer,put failed or lookup failed");
            }
        }
        for (std::size_t i = 0; i != keyCount; ++i) {
            key[3] = static_cast<char>(i);
            map->Remove(key);
        }
        for (std::size_t i = 0; i != keyCount; ++i) {
            key[3] = static_cast<char>(i);
            auto valOpt = map->Lookup(key);
            if (valOpt.Exist()) {
                return this->Fail("Get() return wrong answer,remove failed");
            }
        }
        return this->Success();
    }
};

class WalPersisentTest : public simpletest::ITypenamedTest<WalPersisentTest> {
private:
    using Self = WalPersisentTest;

public:
    WalPersisentTest() noexcept = default;

    ~WalPersisentTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        std::unique_ptr<sharpen::IStatusMap> map{new (std::nothrow) sharpen::WalStatusMap{walName}};
        sharpen::ByteBuffer key{"key", 3};
        sharpen::ByteBuffer value{"val", 3};
        map->Write(key, value);
        map.reset(nullptr);
        map.reset(new (std::nothrow) sharpen::WalStatusMap{walName});
        auto valOpt = map->Lookup(key);
        return this->Assert(valOpt.Exist() && valOpt.Get() == value,
                            "Get() return wrong answer,persistence failed");
    }
};

static int Test() {
    simpletest::TestRunner runner;
    runner.Register<CowMapTest>();
    runner.Register<CowPersistentTest>();
    runner.Register<WalMapTest>();
    runner.Register<WalPersisentTest>();
    int code{runner.Run()};
    sharpen::RemoveFile(cowName);
    sharpen::RemoveFile(walName);
    return code;
}

int main(int argc, char const *argv[]) {
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    return engine.StartupWithCode(&Test);
}