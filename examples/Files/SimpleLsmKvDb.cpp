#include <cstdio>
#include <iterator>
#include <unordered_set>
#include <sharpen/LevelTable.hpp>
#include <sharpen/EventEngine.hpp>
#include <sharpen/IInputPipeChannel.hpp>
#include <sharpen/FileOps.hpp>

void PrintBuffer(const sharpen::ByteBuffer &buf)
{
    for (auto begin = buf.Begin(),end = buf.End(); begin != end; ++begin)
    {
        std::putchar(*begin);   
    }
}

void Entry(const char *dbName)
{
    std::string dir{"./"};
    dir += dbName;
    sharpen::MakeDirectory(dir.c_str());
    dir += "/";
    dir += dbName;
    sharpen::LevelTable table{sharpen::EventEngine::GetEngine(),dir,"kdb"};
    std::puts("input quit to exist db");
    std::puts("get <key> - get a value from databse\n"
            "put <key> <value> - put a key and value to database\n"
            "delete <key> - delete key from database\n"
            "list - list all keys and values from database\n"
            "test - test lsm tree\n"
            "quit - exit database");
    sharpen::InputPipeChannelPtr input = sharpen::MakeStdinPipe();
    input->Register(sharpen::EventEngine::GetEngine());
    while (1)
    {
        std::putchar('>');
        std::putchar(0x02);
        std::string line{input->GetsAsync()};
        if (line == "quit")
        {
            return;   
        }
        if(line == "list")
        {
            std::unordered_set<sharpen::ByteBuffer> set;
            for (auto begin = table.GetMemoryTable().Begin(),end = table.GetMemoryTable().End(); begin != end; ++begin)
            {
                set.emplace(begin->first);
                if(!begin->second.IsDeleted())
                {
                    PrintBuffer(begin->first);
                    std::putchar(':');
                    PrintBuffer(begin->second.Value());
                    std::putchar('\n');
                }
            }
            using MemTable = sharpen::LevelTable::MemTable;
            std::vector<const MemTable*> imms;
            table.GetAllImmutableTables(std::back_inserter(imms));
            for (auto begin = imms.begin(),end = imms.end(); begin != end; ++begin)
            {
                for (auto kb = (*begin)->Begin(),ke = (*begin)->End(); kb != ke; ++kb)
                {
                    auto pair{set.emplace(kb->first)};
                    if (pair.second && !kb->second.IsDeleted())
                    {
                        PrintBuffer(kb->first);
                        std::putchar(':');
                        PrintBuffer(kb->second.Value());
                        std::putchar('\n');
                    }
                }
            }
            for (sharpen::Size i = 0,levels = table.GetMaxLevel(); i != levels; ++i)
            {
                std::vector<const sharpen::LevelView*> views;
                table.GetAllViewOfComponent(i,std::back_inserter(views));
                for (auto vb = views.begin(),ve = views.end(); vb != ve; ++vb)
                {
                    for (auto tb = (*vb)->Begin(),te = (*vb)->End(); tb != te; ++tb)
                    {
                        auto sst{table.GetTable(tb->GetId())};
                        std::vector<sharpen::FilePointer> blocks;
                        sst->TableScan(std::back_inserter(blocks));
                        for (auto bb = blocks.begin(),be = blocks.end(); bb != be; ++bb)
                        {
                            auto block{sst->LoadBlock(*bb)};
                            for (auto begin = block.TwoWayBegin(),end = block.TwoWayEnd(); begin != end; ++begin)
                            {
                                auto pair{set.emplace(begin->GetKey())};
                                if(pair.second && !begin->Value().Empty())
                                {
                                    PrintBuffer(begin->GetKey());
                                    std::putchar(':');
                                    PrintBuffer(begin->Value());
                                    std::putchar('\n');
                                }
                            }
                        }
                    }
                }
            }
           continue;
        }
        if(line == "test")
        {
            std::puts("test put");
            for (sharpen::Size i = 0,count = static_cast<sharpen::Size>(1e6); i < count; ++i)
            {
                sharpen::ByteBuffer key{sizeof(i)};
                key.As<sharpen::Size>() = i;
                sharpen::ByteBuffer value{sizeof(i)};
                value.As<sharpen::Size>() = i;
                table.Put(std::move(key),std::move(value));
            }
            for (sharpen::Size i = 0,count = static_cast<sharpen::Size>(1e6); i < count; ++i)
            {
                sharpen::ByteBuffer key{sizeof(i)};
                key.As<sharpen::Size>() = i;
                sharpen::ByteBuffer value{sizeof(i)};
                value.As<sharpen::Size>() = i;
                if(table.Get(key) != value)
                {
                    std::fputs("bad\n",stderr);
                }
            }
            std::puts("test delete");
            for (sharpen::Size i = 0,count = static_cast<sharpen::Size>(1e6); i < count; ++i)
            {
                sharpen::ByteBuffer key{sizeof(i)};
                key.As<sharpen::Size>() = i;
                table.Delete(key);
            }
            for (sharpen::Size i = 0,count = static_cast<sharpen::Size>(1e6); i < count; ++i)
            {
                sharpen::ByteBuffer key{sizeof(i)};
                key.As<sharpen::Size>() = i;
                if(table.Exist(key) == sharpen::ExistStatus::Exist)
                {
                    std::fputs("bad\n",stderr);
                }
            }
            std::puts("ok");
            continue;
        }
        auto pos = line.find(' ');
        if(pos == line.npos)
        {
            std::puts("unknown command");
            continue;
        }
        std::string command{line.begin(),line.begin() + pos};
        if(command == "get")
        {
            sharpen::ByteBuffer key{line.data() + pos + 1,line.size() - pos - 1};
            auto val{table.TryGet(key)};
            if (!val.Exist())
            {
                std::puts("not found key");
                continue;
            }
            PrintBuffer(key);
            std::putchar(':');
            PrintBuffer(val.Get());
            std::putchar('\n');
        }
        else if(command == "delete")
        {
            sharpen::ByteBuffer key{line.data() + pos + 1,line.size() - pos - 1};
            table.Delete(key);
            std::puts("ok");
        }
        else if(command == "put")
        {
            if(line.size() == pos)
            {
                std::puts("unknown command");
                continue;
            }
            auto kpos = line.find(' ',pos + 1);
            if(pos == line.npos)
            {
                std::puts("unknown command");
                continue;
            }
            sharpen::ByteBuffer key{line.data() + pos + 1,kpos - pos - 1};
            sharpen::ByteBuffer value{line.data() + kpos + 1,line.size() - kpos - 1};
            table.Put(std::move(key),std::move(value));
            std::puts("ok");
        }
        else
        {
            std::puts("unknown command");
        }
    }
}

int main(int argc, char const *argv[])
{
    const char *dbName{nullptr};
    if(argc > 1)
    {
        dbName = argv[1];
    }
    else
    {
        std::fputs("usage: <database name> - create or open datebase\n",stderr);
        return -1;
    }
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    engine.Startup(&Entry,dbName);
    return 0;
}