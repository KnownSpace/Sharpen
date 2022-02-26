#include <cstdio>
#include <cassert>
#include <sharpen/SortedStringTable.hpp>
#include <sharpen/MemoryTable.hpp>
#include <sharpen/BinaryLogger.hpp>
#include <sharpen/FileOps.hpp>
#include <sharpen/EventEngine.hpp>
#include <sharpen/BalancedTable.hpp>

void Entry()
{
    const char *logName = "./wal.log";
    const char *tableName = "./1.table";
    const char *tableName2 = "./2.table";
    const char *tableName3 = "./3.table";
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
            sharpen::SortedStringTable pt{table,mt.Begin(),mt.End(),10,true};
        }
        {
            sharpen::SortedStringTable pt{table};
            auto r = pt.TryGet(sharpen::ByteBuffer{"key_a",5});
            assert(r.Exist());
            assert(r.Get() == sharpen::ByteBuffer("val",3));
            r = pt.TryGet(sharpen::ByteBuffer{"key_b",5});
            assert(r.Exist());
            assert(r.Get() == sharpen::ByteBuffer("val",3));
            r = pt.TryGet(sharpen::ByteBuffer{"other",5});
            assert(r.Exist());
            assert(r.Get() == sharpen::ByteBuffer("val",3));
            r = pt.TryGet(sharpen::ByteBuffer{"abc",3});
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
            std::vector<sharpen::SortedStringTable> pts;
            pts.emplace_back(table);
            pts.emplace_back(table2,mt.Begin(),mt.End(),10,true);
            sharpen::SortedStringTable pt3{table3,pts.begin(),pts.end(),10,true,false};
        }
        {
            sharpen::SortedStringTable pt{table3};
            auto r = pt.TryGet(sharpen::ByteBuffer{"key_a",5});
            assert(r.Exist());
            assert(r.Get() == sharpen::ByteBuffer("val",3));
            r = pt.TryGet(sharpen::ByteBuffer{"key_b",5});
            assert(r.Exist());
            assert(r.Get() == sharpen::ByteBuffer("val",3));
            r = pt.TryGet(sharpen::ByteBuffer{"key_c",5});
            assert(r.Exist());
            assert(r.Get() == sharpen::ByteBuffer("val",3));
            r = pt.TryGet(sharpen::ByteBuffer{"key_d",5});
            assert(r.Exist());
            assert(r.Get() == sharpen::ByteBuffer("val",3));
            r = pt.TryGet(sharpen::ByteBuffer{"other",5});
            assert(r.Exist());
            assert(r.Get() == sharpen::ByteBuffer("val",3));
            r = pt.TryGet(sharpen::ByteBuffer{"abc",3});
            assert(r.Exist());
            assert(r.Get() == sharpen::ByteBuffer("val",3));
        }
        table->Truncate();
        {
            sharpen::BalancedTable pt{table,3};
            {
                sharpen::ByteBuffer key{"key1",4};
                sharpen::ByteBuffer value{"val",3};
                pt.Put(key,value);
            }
            {
                sharpen::ByteBuffer key{"key2",4};
                sharpen::ByteBuffer value{"val",3};
                pt.Put(key,value);
            }
            {
                sharpen::ByteBuffer key{"key3",4};
                sharpen::ByteBuffer value{"val",3};
                pt.Put(key,value);
            }
            {
                sharpen::ByteBuffer key{"key4",4};
                sharpen::ByteBuffer value{"val",3};
                pt.Put(key,value);
            }
            {
                sharpen::ByteBuffer key{"key5",4};
                sharpen::ByteBuffer value{"val",3};
                pt.Put(key,value);
            }
            {
                sharpen::ByteBuffer key{"key6",4};
                sharpen::ByteBuffer value{"val",3};
                pt.Put(key,value);
            }
            {
                sharpen::ByteBuffer key{"key7",4};
                sharpen::ByteBuffer value{"val",3};
                pt.Put(key,value);
            }
            {
                sharpen::ByteBuffer key{"key8",4};
                sharpen::ByteBuffer value{"val",3};
                pt.Put(key,value);
            } 
            assert(pt.GetDepth() == 2);
        }
        {
            sharpen::BalancedTable pt{table,3};
            assert(pt.GetDepth() == 2);
            {
                sharpen::ByteBuffer key{"key1",4};
                sharpen::ByteBuffer value{"val",3};
                assert(pt.Get(key) == value);
            }
            {
                sharpen::ByteBuffer key{"key2",4};
                sharpen::ByteBuffer value{"val",3};
                assert(pt.Get(key) == value);
            }
            {
                sharpen::ByteBuffer key{"key3",4};
                sharpen::ByteBuffer value{"val",3};
                assert(pt.Get(key) == value);
            }
            {
                sharpen::ByteBuffer key{"key4",4};
                sharpen::ByteBuffer value{"val",3};
                assert(pt.Get(key) == value);
            }
            {
                sharpen::ByteBuffer key{"key5",4};
                sharpen::ByteBuffer value{"val",3};
                assert(pt.Get(key) == value);
            }
            {
                sharpen::ByteBuffer key{"key6",4};
                sharpen::ByteBuffer value{"val",3};
                assert(pt.Get(key) == value);
            }
            {
                sharpen::ByteBuffer key{"key7",4};
                sharpen::ByteBuffer value{"val",3};
                assert(pt.Get(key) == value);
            }
            {
                sharpen::ByteBuffer key{"key8",4};
                sharpen::ByteBuffer value{"val",3};
                assert(pt.Get(key) == value);
            }
        }
        table->Truncate();
        {
            sharpen::BalancedTable pt{table,3};
            {
                sharpen::ByteBuffer key{"key1",4};
                sharpen::ByteBuffer value{"val",3};
                pt.Put(key,value);
            }
            {
                sharpen::ByteBuffer key{"key2",4};
                sharpen::ByteBuffer value{"val",3};
                pt.Put(key,value);
            }
            {
                sharpen::ByteBuffer key{"key3",4};
                sharpen::ByteBuffer value{"val",3};
                pt.Put(key,value);
            }
            {
                sharpen::ByteBuffer key{"key4",4};
                sharpen::ByteBuffer value{"val",3};
                pt.Put(key,value);
            }
            {
                sharpen::ByteBuffer key{"key1",4};
                pt.Delete(key);
            }
            {
                sharpen::ByteBuffer key{"key2",4};
                pt.Delete(key);
            }
            assert(pt.GetDepth() == 0);
        }
        {
            sharpen::BalancedTable pt{table,3};
            assert(pt.GetDepth() == 0);
            {
                sharpen::ByteBuffer key{"key1",4};
                assert(pt.Exist(key) == sharpen::ExistStatus::NotExist);
            }
            {
                sharpen::ByteBuffer key{"key2",4};
                assert(pt.Exist(key) == sharpen::ExistStatus::NotExist);
            }
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