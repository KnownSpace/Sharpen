#include <cstdio>
#include <cassert>
#include <iterator>
#include <random>
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
    std::mt19937 random;
    try
    {
        std::puts("persistence test begin");
        sharpen::FileChannelPtr log = sharpen::MakeFileChannel(logName, sharpen::FileAccessModel::All, sharpen::FileOpenModel::CreateNew);
        log->Register(sharpen::EventEngine::GetEngine());
        sharpen::FileChannelPtr table = sharpen::MakeFileChannel(tableName, sharpen::FileAccessModel::All, sharpen::FileOpenModel::CreateNew);
        table->Register(sharpen::EventEngine::GetEngine());
        {
            sharpen::MemoryTable<sharpen::BinaryLogger> mt{log};
            mt.Put(sharpen::ByteBuffer{"key_a", 5}, sharpen::ByteBuffer{"val", 3});
            mt.Put(sharpen::ByteBuffer{"key_b", 5}, sharpen::ByteBuffer{"val", 3});
            mt.Put(sharpen::ByteBuffer{"abc", 3}, sharpen::ByteBuffer{"val", 3});
            mt.Put(sharpen::ByteBuffer{"other", 5}, sharpen::ByteBuffer{"val", 3});
            sharpen::SortedStringTable pt{table};
            pt.BuildFromMemory(mt.Begin(),mt.End(),sharpen::SstBuildOption{true});
        }
        {
            sharpen::SortedStringTable pt{table};
            auto r = pt.TryGet(sharpen::ByteBuffer{"key_a", 5});
            assert(r.Exist());
            assert(r.Get() == sharpen::ByteBuffer("val", 3));
            r = pt.TryGet(sharpen::ByteBuffer{"key_b", 5});
            assert(r.Exist());
            assert(r.Get() == sharpen::ByteBuffer("val", 3));
            r = pt.TryGet(sharpen::ByteBuffer{"other", 5});
            assert(r.Exist());
            assert(r.Get() == sharpen::ByteBuffer("val", 3));
            r = pt.TryGet(sharpen::ByteBuffer{"abc", 3});
            assert(r.Exist());
            assert(r.Get() == sharpen::ByteBuffer("val", 3));
            // range query
            std::vector<sharpen::FilePointer> pointers;
            pt.RangeQuery(std::back_inserter(pointers), sharpen::ByteBuffer{"abc", 3}, sharpen::ByteBuffer{"other", 5});
            std::printf("sst range query need to load %zu blocks\n", pointers.size());
            for (auto begin = pointers.begin(), end = pointers.end(); begin != end; ++begin)
            {
                sharpen::SstDataBlock block{pt.LoadBlock(begin->offset_, begin->size_)};
                for (auto keyBegin = block.TwoWayBegin(), keyEnd = block.TwoWayEnd(); keyBegin != keyEnd; ++keyBegin)
                {
                    std::fputs("key is ", stdout);
                    for (sharpen::Size i = 0; i != keyBegin->GetKey().GetSize(); ++i)
                    {
                        std::putchar(keyBegin->GetKey()[i]);
                    }
                    std::fputs(" value is ", stdout);
                    for (sharpen::Size i = 0; i != keyBegin->Value().GetSize(); ++i)
                    {
                        std::putchar(keyBegin->Value()[i]);
                    }
                    std::putchar('\n');
                }
            }
            // range query
            pointers.clear();
            pt.RangeQuery(std::back_inserter(pointers), sharpen::ByteBuffer{"othera", 6}, sharpen::ByteBuffer{"otherz", 6});
            std::printf("sst range query need to load %zu blocks\n", pointers.size());
            for (auto begin = pointers.begin(), end = pointers.end(); begin != end; ++begin)
            {
                sharpen::SstDataBlock block{pt.LoadBlock(begin->offset_, begin->size_)};
                for (auto keyBegin = block.TwoWayBegin(), keyEnd = block.TwoWayEnd(); keyBegin != keyEnd; ++keyBegin)
                {

                    std::fputs("key is ", stdout);
                    for (sharpen::Size i = 0; i != keyBegin->GetKey().GetSize(); ++i)
                    {
                        std::putchar(keyBegin->GetKey()[i]);
                    }
                    std::fputs(" value is ", stdout);
                    for (sharpen::Size i = 0; i != keyBegin->Value().GetSize(); ++i)
                    {
                        std::putchar(keyBegin->Value()[i]);
                    }
                    std::putchar('\n');
                }
            }
        }
        table->Truncate();
        {
            sharpen::MemoryTable<sharpen::BinaryLogger> mt{log};
            for (sharpen::Uint32 i = 0,count = 114515; i != count;++i)
            {
                sharpen::ByteBuffer key{sizeof(sharpen::Uint32)};
                key.As<sharpen::Uint32>() = i;
                sharpen::ByteBuffer value{sizeof(sharpen::Uint32)};
                value.As<sharpen::Uint32>() = i;
                mt.Put(std::move(key),std::move(value));
            }
            sharpen::SortedStringTable pt{table};
            pt.BuildFromMemory(mt.Begin(),mt.End(),sharpen::SstBuildOption{true});
        }
        {
            sharpen::SortedStringTable pt{table};
            for (sharpen::Uint32 i = 0,count = 114515; i != count;++i)
            {
                sharpen::ByteBuffer key{sizeof(sharpen::Uint32)};
                key.As<sharpen::Uint32>() = i;
                sharpen::ByteBuffer value{sizeof(sharpen::Uint32)};
                value.As<sharpen::Uint32>() = i;
                std::printf("sst check %u/%u\n",i,count - 1);
                assert(pt.Get(key) == value);
            }
        }
        table->Truncate();
        {
            sharpen::MemoryTable<sharpen::BinaryLogger> mt{log};
            for (sharpen::Uint32 i = 0,count = 114515; i != count;++i)
            {
                sharpen::ByteBuffer key{sizeof(sharpen::Uint32)};
                key.As<sharpen::Uint32>() = random();
                sharpen::ByteBuffer value{sizeof(sharpen::Uint32)};
                value.As<sharpen::Uint32>() = random();
                mt.Put(std::move(key),std::move(value));
            }
            sharpen::SortedStringTable pt{table};
            pt.BuildFromMemory(mt.Begin(),mt.End(),sharpen::SstBuildOption{true});
        }
        sharpen::FileChannelPtr table2 = sharpen::MakeFileChannel(tableName2, sharpen::FileAccessModel::All, sharpen::FileOpenModel::CreateNew);
        table2->Register(sharpen::EventEngine::GetEngine());
        sharpen::FileChannelPtr table3 = sharpen::MakeFileChannel(tableName3, sharpen::FileAccessModel::All, sharpen::FileOpenModel::CreateNew);
        table3->Register(sharpen::EventEngine::GetEngine());
        {
            sharpen::MemoryTable<sharpen::BinaryLogger> mt{log};
            mt.Restore();
            mt.Put(sharpen::ByteBuffer{"key_c", 5}, sharpen::ByteBuffer{"val", 3});
            mt.Put(sharpen::ByteBuffer{"key_d", 5}, sharpen::ByteBuffer{"val", 3});
            std::vector<sharpen::SortedStringTable> pts;
            pts.emplace_back(table);
            {
                sharpen::SortedStringTable pt2{table2};
                pt2.BuildFromMemory(mt.Begin(),mt.End(),sharpen::SstBuildOption{true});
            }
            pts.emplace_back(table2);
            sharpen::SortedStringTable pt3{table3};
            pt3.MergeFromTables(pts.begin(),pts.end(),sharpen::SstBuildOption{true},false);
        }
        {
            sharpen::SortedStringTable pt{table3};
            auto r = pt.TryGet(sharpen::ByteBuffer{"key_a", 5});
            assert(r.Exist());
            assert(r.Get() == sharpen::ByteBuffer("val", 3));
            r = pt.TryGet(sharpen::ByteBuffer{"key_b", 5});
            assert(r.Exist());
            assert(r.Get() == sharpen::ByteBuffer("val", 3));
            r = pt.TryGet(sharpen::ByteBuffer{"key_c", 5});
            assert(r.Exist());
            assert(r.Get() == sharpen::ByteBuffer("val", 3));
            r = pt.TryGet(sharpen::ByteBuffer{"key_d", 5});
            assert(r.Exist());
            assert(r.Get() == sharpen::ByteBuffer("val", 3));
            r = pt.TryGet(sharpen::ByteBuffer{"other", 5});
            assert(r.Exist());
            assert(r.Get() == sharpen::ByteBuffer("val", 3));
            r = pt.TryGet(sharpen::ByteBuffer{"abc", 3});
            assert(r.Exist());
            assert(r.Get() == sharpen::ByteBuffer("val", 3));
        }
        table->Truncate();
        {
            sharpen::BalancedTable pt{table,sharpen::BtOption{3}};
            {
                sharpen::ByteBuffer key{"key1", 4};
                sharpen::ByteBuffer value{"val", 3};
                pt.Put(key, value);
            }
            {
                sharpen::ByteBuffer key{"key2", 4};
                sharpen::ByteBuffer value{"val", 3};
                pt.Put(key, value);
            }
            {
                sharpen::ByteBuffer key{"key3", 4};
                sharpen::ByteBuffer value{"val", 3};
                pt.Put(key, value);
            }
            {
                sharpen::ByteBuffer key{"key4", 4};
                sharpen::ByteBuffer value{"val", 3};
                pt.Put(key, value);
            }
            {
                sharpen::ByteBuffer key{"key5", 4};
                sharpen::ByteBuffer value{"val", 3};
                pt.Put(key, value);
            }
            {
                sharpen::ByteBuffer key{"key6", 4};
                sharpen::ByteBuffer value{"val", 3};
                pt.Put(key, value);
            }
            {
                sharpen::ByteBuffer key{"key7", 4};
                sharpen::ByteBuffer value{"val", 3};
                pt.Put(key, value);
            }
            {
                sharpen::ByteBuffer key{"key8", 4};
                sharpen::ByteBuffer value{"val", 3};
                pt.Put(key, value);
            }
            assert(pt.GetDepth() == 2);
        }
        {
            sharpen::BalancedTable pt{table,sharpen::BtOption{3}};
            assert(pt.GetDepth() == 2);
            {
                sharpen::ByteBuffer key{"key1", 4};
                sharpen::ByteBuffer value{"val", 3};
                assert(pt.Get(key) == value);
            }
            {
                sharpen::ByteBuffer key{"key2", 4};
                sharpen::ByteBuffer value{"val", 3};
                assert(pt.Get(key) == value);
            }
            {
                sharpen::ByteBuffer key{"key3", 4};
                sharpen::ByteBuffer value{"val", 3};
                assert(pt.Get(key) == value);
            }
            {
                sharpen::ByteBuffer key{"key4", 4};
                sharpen::ByteBuffer value{"val", 3};
                assert(pt.Get(key) == value);
            }
            {
                sharpen::ByteBuffer key{"key5", 4};
                sharpen::ByteBuffer value{"val", 3};
                assert(pt.Get(key) == value);
            }
            {
                sharpen::ByteBuffer key{"key6", 4};
                sharpen::ByteBuffer value{"val", 3};
                assert(pt.Get(key) == value);
            }
            {
                sharpen::ByteBuffer key{"key7", 4};
                sharpen::ByteBuffer value{"val", 3};
                assert(pt.Get(key) == value);
            }
            {
                sharpen::ByteBuffer key{"key8", 4};
                sharpen::ByteBuffer value{"val", 3};
                assert(pt.Get(key) == value);
            }
            {
                sharpen::ByteBuffer beginKey{"key1", 4};
                sharpen::ByteBuffer endKey{"key8", 4};
                std::vector<sharpen::FilePointer> pointers;
                pt.RangeQuery(std::back_inserter(pointers), beginKey, endKey);
                std::printf("bt range query need to load %zu blocks\n", pointers.size());
                for (auto begin = pointers.begin(), end = pointers.end(); begin != end; ++begin)
                {
                    sharpen::BtBlock block{pt.LoadBlock(begin->offset_, begin->size_)};
                    for (auto keyBegin = block.Begin(), keyEnd = block.End(); keyBegin != keyEnd; ++keyBegin)
                    {
                        std::fputs("key is ", stdout);
                        for (sharpen::Size i = 0; i != keyBegin->GetKey().GetSize(); ++i)
                        {
                            std::putchar(keyBegin->GetKey()[i]);
                        }
                        std::fputs(" value is ", stdout);
                        for (sharpen::Size i = 0; i != keyBegin->Value().GetSize(); ++i)
                        {
                            std::putchar(keyBegin->Value()[i]);
                        }
                        std::putchar('\n');
                    }
                }
                assert(pt.IsFault() == false);
            }
        }
        table->Truncate();
        {
            sharpen::BalancedTable pt{table,sharpen::BtOption{3}};
            {
                sharpen::ByteBuffer key{"key1", 4};
                sharpen::ByteBuffer value{"val", 3};
                pt.Put(key, value);
            }
            {
                sharpen::ByteBuffer key{"key2", 4};
                sharpen::ByteBuffer value{"val", 3};
                pt.Put(key, value);
            }
            {
                sharpen::ByteBuffer key{"key4", 4};
                sharpen::ByteBuffer value{"value", 5};
                pt.Put(key, value);
            }
            {
                sharpen::ByteBuffer key{"key3", 4};
                sharpen::ByteBuffer value{"val", 3};
                pt.Put(key, value);
            }
            {
                sharpen::ByteBuffer key{"key4", 4};
                sharpen::ByteBuffer value{"val", 3};
                pt.Put(key, value);
            }
            {
                sharpen::ByteBuffer key{"key1", 4};
                pt.Delete(key);
            }
            {
                sharpen::ByteBuffer key{"key2", 4};
                pt.Delete(key);
            }
            assert(pt.GetDepth() == 0);
        }
        {
            sharpen::BalancedTable pt{table,sharpen::BtOption{3}};
            assert(pt.GetDepth() == 0);
            {
                sharpen::ByteBuffer key{"key1", 4};
                assert(pt.Exist(key) == sharpen::ExistStatus::NotExist);
            }
            {
                sharpen::ByteBuffer key{"key2", 4};
                assert(pt.Exist(key) == sharpen::ExistStatus::NotExist);
            }
            {
                sharpen::ByteBuffer beginKey{"key1", 4};
                sharpen::ByteBuffer endKey{"key2", 4};
                std::vector<sharpen::FilePointer> pointers;
                pt.RangeQuery(std::back_inserter(pointers), beginKey, endKey);
                std::printf("bt range query need to load %zu blocks\n", pointers.size());
                for (auto begin = pointers.begin(), end = pointers.end(); begin != end; ++begin)
                {
                    sharpen::BtBlock block{pt.LoadBlock(begin->offset_, begin->size_)};
                    for (auto keyBegin = block.Begin(), keyEnd = block.End(); keyBegin != keyEnd; ++keyBegin)
                    {
                        std::fputs("key is ", stdout);
                        for (sharpen::Size i = 0; i != keyBegin->GetKey().GetSize(); ++i)
                        {
                            std::putchar(keyBegin->GetKey()[i]);
                        }
                        std::fputs(" value is ", stdout);
                        for (sharpen::Size i = 0; i != keyBegin->Value().GetSize(); ++i)
                        {
                            std::putchar(keyBegin->Value()[i]);
                        }
                        std::putchar('\n');
                    }
                }
                assert(pt.IsFault() == false);
            }
            table->Truncate();
            {
                sharpen::BalancedTable pt{table,sharpen::BtOption{}};
                for (sharpen::Uint32 i = 0,count = 114515; i != count;++i)
                {
                    sharpen::ByteBuffer key{sizeof(sharpen::Uint32)};
                    key.As<sharpen::Uint32>() = i;
                    sharpen::ByteBuffer value{sizeof(sharpen::Uint32)};
                    value.As<sharpen::Uint32>() = i;
                    pt.Put(std::move(key),std::move(value));
                }
            }
            {
                sharpen::BalancedTable pt{table,sharpen::BtOption{}};
                for (sharpen::Uint32 i = 0,count = 114515; i != count;++i)
                {
                    sharpen::ByteBuffer key{sizeof(sharpen::Uint32)};
                    key.As<sharpen::Uint32>() = i;
                    sharpen::ByteBuffer value{sizeof(sharpen::Uint32)};
                    value.As<sharpen::Uint32>() = i;
                    std::printf("bt check %u/%u\n",i,count - 1);
                    assert(pt.Get(key) == value);
                }
            }
            table->Truncate();
            {
                sharpen::BalancedTable pt{table,sharpen::BtOption{}};
                for (sharpen::Uint32 i = 0,count = 114515; i != count;++i)
                {
                    sharpen::ByteBuffer key{sizeof(sharpen::Uint32)};
                    key.As<sharpen::Uint32>() = random();
                    sharpen::ByteBuffer value{sizeof(sharpen::Uint32)};
                    value.As<sharpen::Uint32>() = random();
                    pt.Put(std::move(key),std::move(value));
                }
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