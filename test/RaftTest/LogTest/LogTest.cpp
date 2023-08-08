#include <sharpen/CowStatusMap.hpp>
#include <sharpen/DebugTools.hpp>
#include <sharpen/EventEngine.hpp>
#include <sharpen/FileOps.hpp>
#include <sharpen/IStatusMap.hpp>
#include <sharpen/WalLogStorage.hpp>
#include <simpletest/TestRunner.hpp>
#include <cinttypes>

static const char *walName = "./walLog";

class LogStorageTest : public simpletest::ITypenamedTest<LogStorageTest> {
private:
    using Self = LogStorageTest;

public:
    LogStorageTest() noexcept = default;

    ~LogStorageTest() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        std::unique_ptr<sharpen::ILogStorage> log{new (std::nothrow)
                                                      sharpen::WalLogStorage{walName}};
        if (!log) {
            return this->Fail("failed to alloc memory");
        }
        simpletest::TestResult result{this->Success()};
        do {
            sharpen::ByteBuffer entire{"entry", 5};
            log->Write(1, entire);
            if (log->GetLastIndex() != 1) {
                result = this->Fail("last index should be 1");
                break;
            }
            for (std::size_t i = 1; i != log->GetLastIndex() + 1; ++i) {
                sharpen::SyncPrintf(
                    "After Write, Index %zu Status %d\n", i, log->Lookup(i).Exist());
            }
            sharpen::LogEntries entires;
            for (std::size_t i = 0; i != 10; ++i) { entires.Push(entire); }
            log->WriteBatch(2, entires);
            if (log->GetLastIndex() != 11) {
                result = this->Fail("last index should be 11");
                break;
            }
            for (std::size_t i = 1; i != log->GetLastIndex() + 1; ++i) {
                sharpen::SyncPrintf(
                    "After WriteBatch, Index %zu Status %d\n", i, log->Lookup(i).Exist());
            }
            log->DropUntil(5);
            for (std::size_t i = 1; i != 5; ++i) {
                if (log->Lookup(i).Exist()) {
                    result = this->Fail("should not exists");
                    break;
                }
            }
            for (std::size_t i = 1; i != log->GetLastIndex() + 1; ++i) {
                sharpen::SyncPrintf("After Drop, Index %zu Status %d\n", i, log->Lookup(i).Exist());
            }
            log->TruncateFrom(6);
            for (std::size_t i = 6; i != 12; ++i) {
                if (log->Lookup(i).Exist()) {
                    result = this->Fail("should not exists");
                    break;
                }
            }
            for (std::size_t i = 1; i != log->GetLastIndex() + 1; ++i) {
                sharpen::SyncPrintf("Index %zu Status %d\n", i, log->Lookup(i).Exist());
            }
            if (log->GetLastIndex() != 5) {
                result = this->Fail("last index should be 5");
                break;
            }
        } while (0);
        log.reset();
        sharpen::RemoveFile(walName);
        return result;
    }
};

class LogStorageBenchmark_1MB : public simpletest::ITypenamedTest<LogStorageBenchmark_1MB> {
private:
    using Self = LogStorageBenchmark_1MB;

public:
    LogStorageBenchmark_1MB() noexcept = default;

    ~LogStorageBenchmark_1MB() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        std::unique_ptr<sharpen::ILogStorage> log{new (std::nothrow)
                                                      sharpen::WalLogStorage{walName}};
        if (!log) {
            return this->Fail("failed to alloc memory");
        }
        constexpr std::size_t GB{static_cast<std::size_t>(1024 * 1024 * 1024)};
        constexpr std::size_t MB{static_cast<std::size_t>(1024 * 1024)};
        constexpr std::size_t KB{static_cast<std::size_t>(1024)};
        sharpen::LogEntries entires;
        constexpr std::size_t totalSize{1 * GB};
        constexpr std::size_t entrySize{1 * MB};
        auto first{std::chrono::system_clock::now()};
        for (std::size_t i = 0; i != totalSize / entrySize; ++i) {
            entires.Push(sharpen::ByteBuffer{entrySize});
        }
        log->WriteBatch(1, entires);
        auto second{std::chrono::system_clock::now()};
        std::int64_t count{
            std::chrono::duration_cast<std::chrono::milliseconds>(second - first).count()};
        std::printf("Using %" PRId64 " ms to write %zu GB entry size %zu MB\n",
                    count,
                    totalSize / GB,
                    entrySize / MB);
        log.reset();
        sharpen::RemoveFile(walName);
        return this->Success();
    }
};

class LogStorageBenchmark_32KB : public simpletest::ITypenamedTest<LogStorageBenchmark_32KB> {
private:
    using Self = LogStorageBenchmark_32KB;

public:
    LogStorageBenchmark_32KB() noexcept = default;

    ~LogStorageBenchmark_32KB() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        std::unique_ptr<sharpen::ILogStorage> log{new (std::nothrow)
                                                      sharpen::WalLogStorage{walName}};
        if (!log) {
            return this->Fail("failed to alloc memory");
        }
        constexpr std::size_t GB{static_cast<std::size_t>(1024 * 1024 * 1024)};
        constexpr std::size_t MB{static_cast<std::size_t>(1024 * 1024)};
        constexpr std::size_t KB{static_cast<std::size_t>(1024)};
        sharpen::LogEntries entires;
        constexpr std::size_t totalSize{1 * GB};
        constexpr std::size_t entrySize{32 * KB};
        auto first{std::chrono::system_clock::now()};
        for (std::size_t i = 0; i != totalSize / entrySize; ++i) {
            entires.Push(sharpen::ByteBuffer{entrySize});
        }
        log->WriteBatch(1, entires);
        auto second{std::chrono::system_clock::now()};
        std::int64_t count{
            std::chrono::duration_cast<std::chrono::milliseconds>(second - first).count()};
        std::printf("Using %" PRId64 " ms to write %zu GB entry size %zu KB\n",
                    count,
                    totalSize / GB,
                    entrySize / KB);
        log.reset();
        sharpen::RemoveFile(walName);
        return this->Success();
    }
};

class LogStorageBenchmark_1MB_NOTALLOC
    : public simpletest::ITypenamedTest<LogStorageBenchmark_1MB_NOTALLOC> {
private:
    using Self = LogStorageBenchmark_1MB_NOTALLOC;

public:
    LogStorageBenchmark_1MB_NOTALLOC() noexcept = default;

    ~LogStorageBenchmark_1MB_NOTALLOC() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        std::unique_ptr<sharpen::ILogStorage> log{new (std::nothrow)
                                                      sharpen::WalLogStorage{walName}};
        if (!log) {
            return this->Fail("failed to alloc memory");
        }
        constexpr std::size_t GB{static_cast<std::size_t>(1024 * 1024 * 1024)};
        constexpr std::size_t MB{static_cast<std::size_t>(1024 * 1024)};
        constexpr std::size_t KB{static_cast<std::size_t>(1024)};
        sharpen::LogEntries entires;
        constexpr std::size_t totalSize{1 * GB};
        constexpr std::size_t entrySize{1 * MB};
        for (std::size_t i = 0; i != totalSize / entrySize; ++i) {
            entires.Push(sharpen::ByteBuffer{entrySize});
        }
        auto first{std::chrono::system_clock::now()};
        log->WriteBatch(1, entires);
        auto second{std::chrono::system_clock::now()};
        std::int64_t count{
            std::chrono::duration_cast<std::chrono::milliseconds>(second - first).count()};
        std::printf("Using %" PRId64 " ms to write %zu GB entry size %zu MB\n",
                    count,
                    totalSize / GB,
                    entrySize / MB);
        log.reset();
        sharpen::RemoveFile(walName);
        return this->Success();
    }
};

class LogstorageBenchmark_32KB_NOTALLOC
    : public simpletest::ITypenamedTest<LogstorageBenchmark_32KB_NOTALLOC> {
private:
    using Self = LogstorageBenchmark_32KB_NOTALLOC;

public:
    LogstorageBenchmark_32KB_NOTALLOC() noexcept = default;

    ~LogstorageBenchmark_32KB_NOTALLOC() noexcept = default;

    inline const Self &Const() const noexcept {
        return *this;
    }

    inline virtual simpletest::TestResult Run() noexcept {
        std::unique_ptr<sharpen::ILogStorage> log{new (std::nothrow)
                                                      sharpen::WalLogStorage{walName}};
        if (!log) {
            return this->Fail("failed to alloc memory");
        }
        constexpr std::size_t GB{static_cast<std::size_t>(1024 * 1024 * 1024)};
        constexpr std::size_t MB{static_cast<std::size_t>(1024 * 1024)};
        constexpr std::size_t KB{static_cast<std::size_t>(1024)};
        sharpen::LogEntries entires;
        constexpr std::size_t totalSize{1 * GB};
        constexpr std::size_t entrySize{32 * KB};
        for (std::size_t i = 0; i != totalSize / entrySize; ++i) {
            entires.Push(sharpen::ByteBuffer{entrySize});
        }
        auto first{std::chrono::system_clock::now()};
        log->WriteBatch(1, entires);
        auto second{std::chrono::system_clock::now()};
        std::int64_t count{
            std::chrono::duration_cast<std::chrono::milliseconds>(second - first).count()};
        std::printf("Using %" PRId64 " ms to write %zu GB entry size %zu KB\n",
                    count,
                    totalSize / GB,
                    entrySize / KB);
        log.reset();
        sharpen::RemoveFile(walName);
        return this->Success();
    }
};

static int Test() {
    simpletest::TestRunner runner{simpletest::DisplayMode::Blocked};
    runner.Register<LogStorageTest>();
    runner.Register<LogStorageBenchmark_1MB>();
    runner.Register<LogStorageBenchmark_32KB>();
    runner.Register<LogStorageBenchmark_1MB_NOTALLOC>();
    runner.Register<LogstorageBenchmark_32KB_NOTALLOC>();
    return runner.Run();
}

int main(int argc, char const *argv[]) {
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    return engine.StartupWithCode(&Test);
}