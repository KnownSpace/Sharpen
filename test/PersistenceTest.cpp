#include <cstdio>
#include <cassert>
#include <iterator>
#include <sharpen/SortedStringTable.hpp>
#include <sharpen/MemoryTable.hpp>
#include <sharpen/BinaryLogger.hpp>
#include <sharpen/FileOps.hpp>
#include <sharpen/EventEngine.hpp>
#include <sharpen/BalancedTable.hpp>
#include <sharpen/LevelTable.hpp>

static std::int32_t CompAsUint32(const sharpen::ByteBuffer &left,const sharpen::ByteBuffer &right) noexcept
{
    std::uint32_t l{left.As<std::uint32_t>()};
    std::uint32_t r{right.As<std::uint32_t>()};
    if(l < r)
    {
        return -1;
    }
    else if(l > r)
    {
        return 1;
    }
    return 0;
}


struct MapPred
{
    bool operator()(const sharpen::ByteBuffer &lhs, const sharpen::ByteBuffer &rhs) const 
    {
        return lhs.As<std::uint32_t>() < rhs.As<std::uint32_t>();
    }
};


void Entry()
{
    const char *logName = "./wal.log";
    const char *tableName = "./1.table";
    const char *tableName2 = "./2.table";
    const char *tableName3 = "./3.table";
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
            pt.Build(mt.Begin(),mt.End(),sharpen::SstBuildOption{true});
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
            pt.TableScan(std::back_inserter(pointers), sharpen::ByteBuffer{"abc", 3}, sharpen::ByteBuffer{"other", 5});
            std::printf("sst range query need to load %zu blocks\n", pointers.size());
            // range query
            pointers.clear();
            pt.TableScan(std::back_inserter(pointers), sharpen::ByteBuffer{"othera", 6}, sharpen::ByteBuffer{"otherz", 6});
            std::printf("sst range query need to load %zu blocks\n", pointers.size());
        }
        table->Truncate();
        {
            sharpen::MemoryTable<sharpen::BinaryLogger,MapPred> mt{log};
            for (std::uint32_t i = 0,count = static_cast<std::uint32_t>(1e5); i != count;++i)
            {
                sharpen::ByteBuffer key{sizeof(std::uint32_t)};
                key.As<std::uint32_t>() = i;
                sharpen::ByteBuffer value{sizeof(std::uint32_t)};
                value.As<std::uint32_t>() = i;
                mt.Put(std::move(key),std::move(value));
            }
            sharpen::SortedStringTable pt{table,sharpen::SstOption{&CompAsUint32}};
            pt.Build(mt.Begin(),mt.End(),sharpen::SstBuildOption{true});
        }
        {
            sharpen::SortedStringTable pt{table,sharpen::SstOption{&CompAsUint32}};
            for (std::uint32_t i = 0,count = static_cast<std::uint32_t>(1e5); i != count;++i)
            {
                sharpen::ByteBuffer key{sizeof(std::uint32_t)};
                key.As<std::uint32_t>() = i;
                sharpen::ByteBuffer value{sizeof(std::uint32_t)};
                value.As<std::uint32_t>() = i;
                assert(pt.Get(key) == value);
            }
            {
                std::size_t count{0};
                for (auto blockBegin = pt.Root().IndexBlock().Begin(),blockEnd = pt.Root().IndexBlock().End(); blockBegin != blockEnd; ++blockBegin)
                {
                    sharpen::FilePointer pointer{blockBegin->Block()};
                    sharpen::SstDataBlock block{pt.LoadBlock(pointer)};
                    for (auto begin = block.TwoWayBegin(),end = block.TwoWayEnd(); begin != end; ++begin)
                    {
                        count += 1;
                    }
                }
                std::printf("sst scan %zu keys\n",count);
                assert(count == static_cast<std::uint32_t>(1e5));
            }
        }
        table->Truncate();
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
                pt2.Build(mt.Begin(),mt.End(),sharpen::SstBuildOption{true});
            }
            pts.emplace_back(table2);
            sharpen::SortedStringTable pt3{table3};
            pt3.Merge(pts.begin(),pts.end(),sharpen::SstBuildOption{true},false);
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
                pt.TableScan(std::back_inserter(pointers), beginKey, endKey);
                std::printf("bt range query need to load %zu blocks\n", pointers.size());
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
                pt.TableScan(std::back_inserter(pointers), beginKey, endKey);
                std::printf("bt range query need to load %zu blocks\n", pointers.size());
                assert(pt.IsFault() == false);
            }
            table->Truncate();
            {
                sharpen::BalancedTable pt{table,sharpen::BtOption{CompAsUint32}};
                for (std::uint32_t i = 0,count = static_cast<std::uint32_t>(1e5); i != count;++i)
                {
                    sharpen::ByteBuffer key{sizeof(std::uint32_t)};
                    key.As<std::uint32_t>() = i;
                    sharpen::ByteBuffer value{sizeof(std::uint32_t)};
                    value.As<std::uint32_t>() = i;
                    pt.Put(std::move(key),std::move(value));
                }
            }
            {
                sharpen::BalancedTable pt{table,sharpen::BtOption{CompAsUint32}};
                for (std::uint32_t i = 0,count = static_cast<std::uint32_t>(1e5); i != count;++i)
                {
                    sharpen::ByteBuffer key{sizeof(std::uint32_t)};
                    key.As<std::uint32_t>() = i;
                    sharpen::ByteBuffer value{sizeof(std::uint32_t)};
                    value.As<std::uint32_t>() = i;
                    assert(pt.Get(key) == value);
                }
                {
                    std::size_t count{0};
                    std::vector<sharpen::FilePointer> blocks;
                    pt.TableScan(std::back_inserter(blocks));
                    for (auto blockBegin = blocks.begin(),blockEnd = blocks.end(); blockBegin != blockEnd; ++blockBegin)
                    {
                        sharpen::FilePointer pointer{*blockBegin};
                        sharpen::BtBlock block{pt.LoadBlock(pointer)};
                        for (auto begin = block.Begin(),end = block.End(); begin != end; ++begin)
                        {
                            count += 1;
                        }
                    }
                    std::printf("bt scan %zu keys\n",count);
                }
            }
            table->Truncate();
        }
        log->Close();
        table->Close();
        table2->Close();
        table3->Close();
        sharpen::RemoveFile(logName);
        sharpen::RemoveFile(tableName);
        sharpen::RemoveFile(tableName2);
        sharpen::RemoveFile(tableName3);
        {
            sharpen::LevelTable table{sharpen::EventEngine::GetEngine(),"./table","kdb",sharpen::LevelTableOption{&CompAsUint32}};
            for (std::uint32_t i = 0,count = static_cast<std::uint32_t>(1e5); i != count;++i)
            {
                sharpen::ByteBuffer key{sizeof(std::uint32_t)};
                key.As<std::uint32_t>() = i;
                sharpen::ByteBuffer value{sizeof(std::uint32_t)};
                value.As<std::uint32_t>() = i;
                table.Put(std::move(key),std::move(value));
            }
            for (std::uint32_t i = 0,count = static_cast<std::uint32_t>(1e5); i != count;++i)
            {
                sharpen::ByteBuffer key{sizeof(std::uint32_t)};
                key.As<std::uint32_t>() = i;
                sharpen::ByteBuffer value{sizeof(std::uint32_t)};
                value.As<std::uint32_t>() = i;
                assert(table.Get(key) == value);
            }
            sharpen::ByteBuffer begin{sizeof(std::uint32_t)},end{sizeof(std::uint32_t)};
            begin.As<std::uint32_t>() = 0;
            end.As<std::uint32_t>() = 10000;
            table.Destory();
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