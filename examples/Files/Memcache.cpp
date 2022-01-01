#include <cstdio>
#include <cstring>
#include <map>
#include <sharpen/MemoryTable.hpp>
#include <sharpen/BinaryLogger.hpp>
#include <sharpen/EventEngine.hpp>

void PrintUsage()
{
    std::puts("usage:\n"
                "\tli - list all of <key,value>\n"
                "\tget <key> - get value\n"
                "\tput <key> <value> - put <key,value> to cache\n"
                "\tdelete <key> - delete value");
}

using CacheTable = sharpen::MemoryTable<std::map,sharpen::BinaryLogger<std::map>>;

CacheTable BuildTable(const char *logPath)
{
    sharpen::FileChannelPtr log = sharpen::MakeFileChannel(logPath,sharpen::FileAccessModel::All,sharpen::FileOpenModel::CreateOrOpen);
    log->Register(sharpen::EventEngine::GetEngine());
    return CacheTable{log};
}

void Entry(int argc, char const *argv[])
{
    if(argc < 2)
    {
        PrintUsage();
        return;
    }
    const char *op = argv[1];
    if(!std::strcmp(op,"get"))
    {
        if(argc < 3)
        {
            PrintUsage();
            return;
        }
        CacheTable tb = BuildTable("./binlog.cache");
        tb.RestoreFromLogger();
        sharpen::ByteBuffer key{argv[2],std::strlen(argv[2])};
        if(tb.Exist(key))
        {
            key = tb.Get(key);
            for (auto begin = key.Begin(),end = key.End(); begin != end; ++begin)
            {
                std::putchar(*begin);
            }
            return;
        }
        std::puts("key doesn't exist");
    }
    else if(!std::strcmp(op,"delete"))
    {
        if(argc < 3)
        {
            PrintUsage();
            return;
        }
        CacheTable tb = BuildTable("./binlog.cache");
        tb.RestoreFromLogger();
        sharpen::ByteBuffer key{argv[2],std::strlen(argv[2])};
        if(tb.Exist(key))
        {
            tb.Delete(key);
            std::puts("success");
            return;
        }
        std::puts("key doesn't exist");
    }
    else if(!std::strcmp(op,"put"))
    {
        if(argc < 4)
        {
            PrintUsage();
            return;
        }
        CacheTable tb = BuildTable("./binlog.cache");
        tb.RestoreFromLogger();
        sharpen::ByteBuffer key{argv[2],std::strlen(argv[2])};
        sharpen::ByteBuffer val{argv[3],std::strlen(argv[3])};
        tb.Put(key,val);
        std::puts("success");
    }
    else if(!std::strcmp(op,"li"))
    {
        CacheTable tb = BuildTable("./binlog.cache");
        tb.RestoreFromLogger();
        std::puts("table:");
        for (auto begin = tb.Begin(),end = tb.End(); begin != end; ++begin)
        {
            for (size_t i = 0; i < begin->first.GetSize(); ++i)
            {
                std::putchar(begin->first[i]);
            }
            std::putchar(':');
            for (size_t i = 0; i < begin->second.GetSize(); ++i)
            {
                std::putchar(begin->second[i]);
            }
            std::putchar('\n');
        }
    }
    else
    {
        PrintUsage();
    }
}

int main(int argc, char const *argv[])
{
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    engine.Startup(&Entry,argc,argv);
    return 0;
}
