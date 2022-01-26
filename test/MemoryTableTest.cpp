#include <cstdio>
#include <map>
#include <cassert>
#include <cstring>
#include <sharpen/MemoryTable.hpp>
#include <sharpen/EventEngine.hpp>
#include <sharpen/IFileChannel.hpp>
#include <sharpen/BinaryLogger.hpp>
#include <sharpen/FileOps.hpp>

void Entry()
{
    try
    {
        const char *log = "./binlog.log";
        std::puts("memory table test begin");
        {
            sharpen::MemoryTable<sharpen::BinaryLogger> table{log, sharpen::EventEngine::GetEngine()};
            table.Restore();
            sharpen::ByteBuffer key{"key", 3}, val{"val", 3};
            table.Put(key, val);
            key.Append("123", 3);
            table.Put(key, val);
            table.Delete(key);
        }
        {
            sharpen::MemoryTable<sharpen::BinaryLogger> table{log, sharpen::EventEngine::GetEngine()};
            table.Restore();
            sharpen::ByteBuffer key{"key", 3}, val;
            val = table[key];
            assert(!std::memcmp("val", val.Data(), 3));
            key.Append("123", 3);
            assert(table.Exist(key) == sharpen::MemoryTable<sharpen::BinaryLogger>::ExistStatus::Deleted);
        }
        {
            sharpen::MemoryTable<sharpen::BinaryLogger> table{log, sharpen::EventEngine::GetEngine()};
            table.Restore();
            sharpen::WriteBatch batch;
            sharpen::ByteBuffer key{"key", 3}, val{"val", 3};
            batch.Put(key, std::move(val));
            batch.Delete(key);
            table.Action(batch);
            assert(table.Exist(key) == sharpen::MemoryTable<sharpen::BinaryLogger>::ExistStatus::Deleted);
        }
        {
            sharpen::RemoveFile(log);
        }
        std::puts("pass");
    }
    catch (const std::exception &e)
    {
        std::fprintf(stderr, "%s\n", e.what());
    }
}

int main(int argc, char const *argv[])
{
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    engine.Startup(&Entry);
    return 0;
}
