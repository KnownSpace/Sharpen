#include <cstdio>
#include <sharpen/BalancedTable.hpp>
#include <sharpen/EventEngine.hpp>
#include <sharpen/IInputPipeChannel.hpp>

void Entry()
{
    sharpen::FileChannelPtr dbFile = sharpen::MakeFileChannel("./db.bt",sharpen::FileAccessModel::All,sharpen::FileOpenModel::CreateOrOpen);
    dbFile->Register(sharpen::EventEngine::GetEngine());
    sharpen::BalancedTable table{dbFile};
    std::puts("input quit to exist db");
    sharpen::InputPipeChannelPtr input = sharpen::MakeStdinPipe();
    input->Register(sharpen::EventEngine::GetEngine());
    while (1)
    {
        std::string line{input->GetsAsync()};
        if (line == "quit")
        {
            return;   
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
            for (sharpen::Size i = 0; i != val.Get().GetSize(); ++i)
            {
                std::putchar(val.Get()[i]);
            }
            std::putchar('\n');
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
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    engine.Startup(&Entry);
    return 0;
}