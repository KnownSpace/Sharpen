#include <cstdio>
#include <cstring>
#include <map>
#include <sharpen/MemoryTable.hpp>
#include <sharpen/BinaryLogger.hpp>
#include <sharpen/EventEngine.hpp>

void PrintUsage()
{
    std::puts("usage:\n"
                "\tli - list all of item\n"
                "\tget <name> - get item description\n"
                "\tput <name> <value> - put item\n"
                "\tdelete <name> - delete item");
}

using CacheTable = sharpen::MemoryTable<std::map,sharpen::BinaryLogger<std::map>>;

CacheTable BuildTable(const char *logPath)
{
    sharpen::FileChannelPtr log = sharpen::MakeFileChannel(logPath,sharpen::FileAccessModel::All,sharpen::FileOpenModel::CreateOrOpen);
    log->Register(sharpen::EventEngine::GetEngine());
    return CacheTable{log};
}

const char *dbPath = "./todo.binlog";

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
        CacheTable tb = BuildTable(dbPath);
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
        std::puts("item doesn't exist");
    }
    else if(!std::strcmp(op,"delete"))
    {
        if(argc < 3)
        {
            PrintUsage();
            return;
        }
        CacheTable tb = BuildTable(dbPath);
        tb.RestoreFromLogger();
        sharpen::ByteBuffer key{argv[2],std::strlen(argv[2])};
        if(tb.Exist(key))
        {
            tb.Delete(key);
            std::puts("success");
            return;
        }
        std::puts("item doesn't exist");
    }
    else if(!std::strcmp(op,"put"))
    {
        if(argc < 4)
        {
            PrintUsage();
            return;
        }
        CacheTable tb = BuildTable(dbPath);
        tb.RestoreFromLogger();
        sharpen::ByteBuffer key{argv[2],std::strlen(argv[2])};
        sharpen::ByteBuffer val{argv[3],std::strlen(argv[3])};
        tb.Put(key,val);
        std::puts("success");
    }
    else if(!std::strcmp(op,"li"))
    {
        CacheTable tb = BuildTable(dbPath);
        tb.RestoreFromLogger();
        std::puts("list of items:");
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
