#include <cstdio>
#include <iterator>
#include <sharpen/BalancedTable.hpp>
#include <sharpen/EventEngine.hpp>
#include <sharpen/IInputPipeChannel.hpp>

void PrintBuffer(const sharpen::ByteBuffer &buf)
{
    for (auto begin = buf.Begin(),end = buf.End(); begin != end; ++begin)
    {
        std::putchar(*begin);   
    }
}

void Entry(const char *dbName)
{
    sharpen::FileChannelPtr dbFile = sharpen::MakeFileChannel(dbName,sharpen::FileAccessModel::All,sharpen::FileOpenModel::CreateOrOpen);
    dbFile->Register(sharpen::EventEngine::GetEngine());
    sharpen::BalancedTable table{dbFile};
    std::puts("input quit to exist db");
    std::puts("get <key> - get a value from databse\n"
            "put <key> <value> - put a key and value to database\n"
            "delete <key> - delete key from database\n"
            "list - list all keys and values from database\n"
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
        if (line == "list")
        {
            std::vector<sharpen::FilePointer> blocks;
            table.GetRootLock().LockRead();
            std::unique_lock<sharpen::AsyncReadWriteLock> lock{table.GetRootLock(),std::adopt_lock};
            table.TableScan(std::back_inserter(blocks));
            for (auto blockBegin = blocks.begin(),blockEnd = blocks.end(); blockBegin != blockEnd; ++blockBegin)
            {
                table.GetBlockLock(blockBegin->offset_).LockRead();
                std::unique_lock<sharpen::AsyncReadWriteLock> blockLock{table.GetBlockLock(blockBegin->offset_),std::adopt_lock};
                auto block{table.LoadBlock(*blockBegin)};
                for (auto begin = block.Begin(),end = block.End(); begin != end; ++begin)
                {
                    PrintBuffer(begin->GetKey());
                    std::putchar(':');
                    PrintBuffer(begin->Value());
                    std::putchar('\n');
                }
            }
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
    const char *dbName = "./db.bt";
    if(argc > 1)
    {
        dbName = argv[1];
    }
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    engine.Startup(&Entry,dbName);
    return 0;
}