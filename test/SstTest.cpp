#include <cstdio>
#include <cassert>
#include <sharpen/EventEngine.hpp>
#include <sharpen/IFileChannel.hpp>
#include <sharpen/SortedStringTable.hpp>
#include <sharpen/FileOps.hpp>

void Entry()
{
    const char *name = "./sstable";
    try
    {
        std::puts("sst test begin");
        //has index & meta index
        {
            sharpen::FileChannelPtr sstFile = sharpen::MakeFileChannel(name,sharpen::FileAccessModel::Write,sharpen::FileOpenModel::CreateNew);
            sstFile->Register(sharpen::EventEngine::GetEngine());
            sharpen::SortedStringTable table;
            table.IndexBlock().Blocks().push_back({sharpen::ByteBuffer{"datablock",9},{0,0}});
            table.MetaIndexBlock().Blocks().push_back({sharpen::ByteBuffer{"filter",6},{0,0}});
            table.Store(sstFile,0);
            sstFile->Close();
        }
        {
            sharpen::FileChannelPtr sstFile = sharpen::MakeFileChannel(name,sharpen::FileAccessModel::Read,sharpen::FileOpenModel::Open);
            sstFile->Register(sharpen::EventEngine::GetEngine());
            sharpen::SortedStringTable table;
            table.Load(sstFile);
            sstFile->Close();
            assert(table.IndexBlock().Blocks().size() == table.MetaIndexBlock().Blocks().size() && table.IndexBlock().Blocks().size() == 1);
            assert(table.IndexBlock().Blocks().front().Key() == sharpen::ByteBuffer("datablock",9));
            assert(table.MetaIndexBlock().Blocks().front().Key() == sharpen::ByteBuffer("filter",6));
        }
        //empty
        {
            sharpen::FileChannelPtr sstFile = sharpen::MakeFileChannel(name,sharpen::FileAccessModel::Write,sharpen::FileOpenModel::CreateNew);
            sstFile->Register(sharpen::EventEngine::GetEngine());
            sharpen::SortedStringTable table;
            table.Store(sstFile,0);
            sstFile->Close();
        }
        {
            sharpen::FileChannelPtr sstFile = sharpen::MakeFileChannel(name,sharpen::FileAccessModel::Read,sharpen::FileOpenModel::Open);
            sstFile->Register(sharpen::EventEngine::GetEngine());
            sharpen::SortedStringTable table;
            table.Load(sstFile);
            sstFile->Close();
            assert(table.Footer().IndexBlock().size_ == table.Footer().MetaIndexBlock().size_ && table.Footer().IndexBlock().size_ == 0);
        }
        //has index
        {
            sharpen::FileChannelPtr sstFile = sharpen::MakeFileChannel(name,sharpen::FileAccessModel::Write,sharpen::FileOpenModel::CreateNew);
            sstFile->Register(sharpen::EventEngine::GetEngine());
            sharpen::SortedStringTable table;
            table.IndexBlock().Blocks().push_back({sharpen::ByteBuffer{"datablock",9},{0,0}});
            table.Store(sstFile,0);
            sstFile->Close();
        }
        {
            sharpen::FileChannelPtr sstFile = sharpen::MakeFileChannel(name,sharpen::FileAccessModel::Read,sharpen::FileOpenModel::Open);
            sstFile->Register(sharpen::EventEngine::GetEngine());
            sharpen::SortedStringTable table;
            table.Load(sstFile);
            sstFile->Close();
            assert(table.MetaIndexBlock().Blocks().size() == 0 && table.IndexBlock().Blocks().size() == 1);
            assert(table.IndexBlock().Blocks().front().Key() == sharpen::ByteBuffer("datablock",9));
            assert(table.Footer().MetaIndexBlock().size_ == 0);
        }
        //has meta index
        {
            sharpen::FileChannelPtr sstFile = sharpen::MakeFileChannel(name,sharpen::FileAccessModel::Write,sharpen::FileOpenModel::CreateNew);
            sstFile->Register(sharpen::EventEngine::GetEngine());
            sharpen::SortedStringTable table;
            table.MetaIndexBlock().Blocks().push_back({sharpen::ByteBuffer{"filter",6},{0,0}});
            table.Store(sstFile,0);
            sstFile->Close();
        }
        {
            sharpen::FileChannelPtr sstFile = sharpen::MakeFileChannel(name,sharpen::FileAccessModel::Read,sharpen::FileOpenModel::Open);
            sstFile->Register(sharpen::EventEngine::GetEngine());
            sharpen::SortedStringTable table;
            table.Load(sstFile);
            sstFile->Close();
            assert(table.IndexBlock().Blocks().size() == 0 && table.MetaIndexBlock().Blocks().size() == 1);
            assert(table.MetaIndexBlock().Blocks().front().Key() == sharpen::ByteBuffer("filter",6));
            assert(table.Footer().IndexBlock().size_ == 0);
        }
        sharpen::RemoveFile(name);
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
