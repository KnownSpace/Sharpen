#include <cstdio>
#include <cassert>
#include <sharpen/PersistentTable.hpp>
#include <sharpen/MemoryTable.hpp>
#include <sharpen/BinaryLogger.hpp>
#include <sharpen/FileOps.hpp>
#include <sharpen/EventEngine.hpp>

void Entry()
{
    const char *logName = "./log";
    const char *tableName = "./sst";
    const char *tableName2 = "./sst2";
    const char *tableName3 = "./sst3";
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
        sharpen::FileChannelPtr table2 = sharpen::MakeFileChannel(tableName2,sharpen::FileAccessModel::All,sharpen::FileOpenModel::CreateNew);
        table2->Register(sharpen::EventEngine::GetEngine());
        sharpen::FileChannelPtr table3 = sharpen::MakeFileChannel(tableName3,sharpen::FileAccessModel::All,sharpen::FileOpenModel::CreateNew);
        table3->Register(sharpen::EventEngine::GetEngine());
        {
            sharpen::MemoryTable<sharpen::BinaryLogger> mt{log};
            mt.Restore();
            mt.Put(sharpen::ByteBuffer{"key_c",5},sharpen::ByteBuffer{"val",3});
            mt.Put(sharpen::ByteBuffer{"key_d",5},sharpen::ByteBuffer{"val",3});
            std::vector<sharpen::PersistentTable> pts;
            pts.emplace_back(table);
            pts.emplace_back(table2,mt.Begin(),mt.End(),true);
            sharpen::PersistentTable pt3{table3,pts.begin(),pts.end(),true,false};
        }
        {
            sharpen::PersistentTable pt{table3};
            auto r = pt.TryQuery(sharpen::ByteBuffer{"key_a",5});
            assert(r.Exist());
            assert(r.Get() == sharpen::ByteBuffer("val",3));
            r = pt.TryQuery(sharpen::ByteBuffer{"key_b",5});
            assert(r.Exist());
            assert(r.Get() == sharpen::ByteBuffer("val",3));
            r = pt.TryQuery(sharpen::ByteBuffer{"key_c",5});
            assert(r.Exist());
            assert(r.Get() == sharpen::ByteBuffer("val",3));
            r = pt.TryQuery(sharpen::ByteBuffer{"key_d",5});
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
        table2->Close();
        table3->Close();
        sharpen::RemoveFile(logName);
        sharpen::RemoveFile(tableName);
        sharpen::RemoveFile(tableName2);
        sharpen::RemoveFile(tableName3);
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