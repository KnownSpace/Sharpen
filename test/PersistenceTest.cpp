#include <cstdio>
#include <cassert>
#include <sharpen/PersistentTable.hpp>
#include <sharpen/MemoryTable.hpp>
#include <sharpen/BinaryLogger.hpp>
#include <sharpen/FileOps.hpp>

void Entry()
{
    const char *logName = "./log";
    const char *tableName = "./sst";
    try
    {
        std::puts("persistence test begin");
        sharpen::FileChannelPtr log = sharpen::MakeFileChannel(logName,sharpen::FileAccessModel::All,sharpen::FileOpenModel::CreateNew);
        log->Register(sharpen::EventEngine::GetEngine());
        sharpen::FileChannelPtr table = sharpen::MakeFileChannel(tableName,sharpen::FileAccessModel::All,sharpen::FileOpenModel::CreateNew);
        table->Register(sharpen::EventEngine::GetEngine());
        {    
            sharpen::MemoryTable<sharpen::BinaryLogger> mt{log};
            mt.Put(sharpen::ByteBuffer{"key_a",5},sharpen::ByteBuffer{"val",3});
            mt.Put(sharpen::ByteBuffer{"key_b",5},sharpen::ByteBuffer{"val",3});
            mt.Put(sharpen::ByteBuffer{"abc",3},sharpen::ByteBuffer{"val",3});
            mt.Put(sharpen::ByteBuffer{"other",5},sharpen::ByteBuffer{"val",3});
            sharpen::PersistentTable pt{table,mt.Begin(),mt.End(),true};
        }
        {
            sharpen::PersistentTable pt{table};
            auto r = pt.TryQuery(sharpen::ByteBuffer{"key_a",5});
            assert(r.Exist());
            assert(r.Get() == sharpen::ByteBuffer("val",3));
            r = pt.TryQuery(sharpen::ByteBuffer{"key_b",5});
            assert(r.Exist());
            assert(r.Get() == sharpen::ByteBuffer("val",3));
            r = pt.TryQuery(sharpen::ByteBuffer{"other",5});
            assert(r.Exist());
            assert(r.Get() == sharpen::ByteBuffer("val",3));
            r = pt.TryQuery(sharpen::ByteBuffer{"abc",3});
            assert(r.Exist());
            assert(r.Get() == sharpen::ByteBuffer("val",3));
        }
        log->Close();
        table->Close();
        sharpen::RemoveFile(logName);
        sharpen::RemoveFile(tableName);
        std::puts("pass");
    }
    catch(const std::exception& e)
    {
        std::fprintf(stderr,"%s\n",e.what());
    }
}

int main(int argc, char const *argv[])
{
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    engine.Startup(&Entry);   
    return 0;
}