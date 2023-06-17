#include <sharpen/CowStatusMap.hpp>
#include <sharpen/DebugTools.hpp>
#include <sharpen/EventEngine.hpp>
#include <sharpen/FileOps.hpp>
#include <sharpen/IStatusMap.hpp>
#include <sharpen/WalLogStorage.hpp>
#include <simpletest/TestRunner.hpp>


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

static int Test() {
    simpletest::TestRunner runner{simpletest::DisplayMode::Blocked};
    runner.Register<LogStorageTest>();
    return runner.Run();
}

int main(int argc, char const *argv[]) {
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    return engine.StartupWithCode(&Test);
}