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

using CacheTable = sharpen::MemoryTable<sharpen::BinaryLogger>;

CacheTable BuildTable(const char *logPath)
{
    return CacheTable{logPath,sharpen::EventEngine::GetEngine()};
}

const char *dbPath = "./binlog";

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
        tb.Restore();
        sharpen::ByteBuffer key{argv[2],std::strlen(argv[2])};
        if(tb.Exist(key) == CacheTable::ExistStatus::Exist)
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
        tb.Restore();
        sharpen::ByteBuffer key{argv[2],std::strlen(argv[2])};
        if(tb.Exist(key) == CacheTable::ExistStatus::Exist)
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
        tb.Restore();
        sharpen::ByteBuffer key{argv[2],std::strlen(argv[2])};
        sharpen::ByteBuffer val{argv[3],std::strlen(argv[3])};
        tb.Put(key,val);
        std::puts("success");
    }
    else if(!std::strcmp(op,"li"))
    {
        CacheTable tb = BuildTable(dbPath);
        tb.Restore();
        std::puts("list of items:");
        for (auto begin = tb.Begin(),end = tb.End(); begin != end; ++begin)
        {
            //skip deleted key
            if(begin->second.IsDeleted())
            {
                continue;
            }
            for (size_t i = 0; i < begin->first.GetSize(); ++i)
            {
                std::putchar(begin->first[i]);
            }
            std::putchar(':');
            for (size_t i = 0; i < begin->second.Value().GetSize(); ++i)
            {
                std::putchar(begin->second.Value()[i]);
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
