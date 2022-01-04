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
    std::puts("memory table test begin");
    sharpen::FileChannelPtr logFile = sharpen::MakeFileChannel("binlog",sharpen::FileAccessModel::All,sharpen::FileOpenModel::CreateNew);
    logFile->Register(sharpen::EventEngine::GetEngine());
    {
        sharpen::MemoryTable<std::map,sharpen::BinaryLogger<std::map>> table{logFile};
        table.Restore();
        sharpen::ByteBuffer key{"key",3},val{"val",3};
        table.Put(key,val);
        key.Append("123",3);
        table.Put(key,val);
        table.Delete(key);
    }
    {
        sharpen::MemoryTable<std::map,sharpen::BinaryLogger<std::map>> table{logFile};
        table.Restore();
        sharpen::ByteBuffer key{"key",3},val;
        val = table[key];
        assert(!std::memcmp("val",val.Data(),3));
        key.Append("123",3);
        assert(!table.Exist(key));
    }
    {
        sharpen::MemoryTable<std::map,sharpen::BinaryLogger<std::map>> table{logFile};
        table.Restore();
        sharpen::WriteBatch batch;
        sharpen::ByteBuffer key{"key",3},val{"val",3};
        batch.Put(key,std::move(val));
        batch.Delete(key);
        table.Action(batch);
        assert(!table.Exist(key));
    }
    logFile->Close();
    sharpen::RemoveFile("./binlog");
    std::puts("pass");
}

int main(int argc, char const *argv[])
{
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    engine.Startup(&Entry);
    return 0;
}
