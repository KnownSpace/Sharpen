#include <cstdio>
#include <cassert>
#include <sharpen/EventEngine.hpp>
#include <sharpen/IFileChannel.hpp>
#include <sharpen/SortedStringTable.hpp>
#include <sharpen/FileOps.hpp>
#include <sharpen/SstDataBlock.hpp>

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
            table.IndexBlock().Put({sharpen::ByteBuffer{"datablock",9},{0,0}});
            table.MetaIndexBlock().Put({sharpen::ByteBuffer{"filter",6},{0,0}});
            table.StoreTo(sstFile,0);
            sstFile->Close();
        }
        {
            sharpen::FileChannelPtr sstFile = sharpen::MakeFileChannel(name,sharpen::FileAccessModel::Read,sharpen::FileOpenModel::Open);
            sstFile->Register(sharpen::EventEngine::GetEngine());
            sharpen::SortedStringTable table;
            table.LoadFrom(sstFile);
            sstFile->Close();
            assert(table.IndexBlock().Blocks().size() == table.MetaIndexBlock().Blocks().size() && table.IndexBlock().Blocks().size() == 1);
            assert(table.IndexBlock().Blocks().front().GetKey() == sharpen::ByteBuffer("datablock",9));
            assert(table.MetaIndexBlock().Blocks().front().GetKey() == sharpen::ByteBuffer("filter",6));
        }
        //empty
        {
            sharpen::FileChannelPtr sstFile = sharpen::MakeFileChannel(name,sharpen::FileAccessModel::Write,sharpen::FileOpenModel::CreateNew);
            sstFile->Register(sharpen::EventEngine::GetEngine());
            sharpen::SortedStringTable table;
            table.StoreTo(sstFile,0);
            sstFile->Close();
        }
        {
            sharpen::FileChannelPtr sstFile = sharpen::MakeFileChannel(name,sharpen::FileAccessModel::Read,sharpen::FileOpenModel::Open);
            sstFile->Register(sharpen::EventEngine::GetEngine());
            sharpen::SortedStringTable table;
            table.LoadFrom(sstFile);
            sstFile->Close();
            assert(table.Footer().IndexBlock().size_ == table.Footer().MetaIndexBlock().size_ && table.Footer().IndexBlock().size_ == 0);
        }
        //has index
        {
            sharpen::FileChannelPtr sstFile = sharpen::MakeFileChannel(name,sharpen::FileAccessModel::Write,sharpen::FileOpenModel::CreateNew);
            sstFile->Register(sharpen::EventEngine::GetEngine());
            sharpen::SortedStringTable table;
            table.IndexBlock().Put({sharpen::ByteBuffer{"datablock",9},{0,0}});
            table.StoreTo(sstFile,0);
            sstFile->Close();
        }
        {
            sharpen::FileChannelPtr sstFile = sharpen::MakeFileChannel(name,sharpen::FileAccessModel::Read,sharpen::FileOpenModel::Open);
            sstFile->Register(sharpen::EventEngine::GetEngine());
            sharpen::SortedStringTable table;
            table.LoadFrom(sstFile);
            sstFile->Close();
            assert(table.IndexBlock().Blocks().front().GetKey() == sharpen::ByteBuffer("datablock",9));
            assert(table.Footer().MetaIndexBlock().size_ == 0);
        }
        //has meta index
        {
            sharpen::FileChannelPtr sstFile = sharpen::MakeFileChannel(name,sharpen::FileAccessModel::Write,sharpen::FileOpenModel::CreateNew);
            sstFile->Register(sharpen::EventEngine::GetEngine());
            sharpen::SortedStringTable table;
            table.MetaIndexBlock().Put({sharpen::ByteBuffer{"filter",6},{0,0}});
            table.StoreTo(sstFile,0);
            sstFile->Close();
        }
        {
            sharpen::FileChannelPtr sstFile = sharpen::MakeFileChannel(name,sharpen::FileAccessModel::Read,sharpen::FileOpenModel::Open);
            sstFile->Register(sharpen::EventEngine::GetEngine());
            sharpen::SortedStringTable table;
            table.LoadFrom(sstFile);
            sstFile->Close();
            assert(table.IndexBlock().Blocks().size() == 0 && table.MetaIndexBlock().Blocks().size() == 1);
            assert(table.MetaIndexBlock().Blocks().front().GetKey() == sharpen::ByteBuffer("filter",6));
            assert(table.Footer().IndexBlock().size_ == 0);
        }
        sharpen::RemoveFile(name);
        //key value pair test
        {
            sharpen::ByteBuffer buf;
            {
                sharpen::SstKeyValuePair pair{0,5,sharpen::ByteBuffer{"mykey",5},sharpen::ByteBuffer{"myval",5}};
                pair.StoreTo(buf);
            }
            {
                sharpen::SstKeyValuePair pair;
                pair.LoadFrom(buf);
                assert(pair.GetKey() == sharpen::ByteBuffer("mykey",5));
                assert(pair.Value() == sharpen::ByteBuffer("myval",5));
            }
        }
        //key value group test
        {
            sharpen::ByteBuffer buf;
            {
                sharpen::SstKeyValueGroup group;
                group.Put(sharpen::ByteBuffer{"mykey",5},sharpen::ByteBuffer{"myval",5});
                group.Put(sharpen::ByteBuffer{"mykey1",6},sharpen::ByteBuffer{"myval",5});
                assert(!group.TryPut(sharpen::ByteBuffer("oykey",5),sharpen::ByteBuffer("myval",5)));
                group.Put(sharpen::ByteBuffer{"myke",4},sharpen::ByteBuffer{"myval",5});
                group.StoreTo(buf);
            }
            {
                sharpen::SstKeyValueGroup group;
                group.LoadFrom(buf);
                assert(group.GetSize() == 3);
                assert(group[0].GetKey() == sharpen::ByteBuffer("myke",4));
                assert(group[1].GetKey() == sharpen::ByteBuffer("mykey",5));
                assert(group[2].GetKey() == sharpen::ByteBuffer("mykey1",6));
                for (sharpen::Size i = 0; i < group.GetSize(); ++i)
                {
                    assert(group[i].Value() == sharpen::ByteBuffer("myval",5));
                }
            }
        }
        //data block test
        {
            sharpen::ByteBuffer buf;
            {
                sharpen::SstDataBlock block;
                block.Put(sharpen::ByteBuffer{"key_a",5},sharpen::ByteBuffer{"myval",5});
                block.Put(sharpen::ByteBuffer{"key_b",5},sharpen::ByteBuffer{"myval",5});
                block.Put(sharpen::ByteBuffer{"key_c",5},sharpen::ByteBuffer{"myval",5});
                block.Put(sharpen::ByteBuffer{"key_d",5},sharpen::ByteBuffer{"myval",5});
                block.Put(sharpen::ByteBuffer{"key_",4},sharpen::ByteBuffer{"myval",5});
                block.Put(sharpen::ByteBuffer{"other",5},sharpen::ByteBuffer{"myval",5});
                assert(block.GetSize() == 2);
                assert(block[0].GetSize() == 5);
                assert(block[1].GetSize() == 1);
                assert(block[0][0].GetKey() == sharpen::ByteBuffer("key_",4));
                assert(block[0][1].GetKey() == sharpen::ByteBuffer("key_a",5));
                assert(block[0][2].GetKey() == sharpen::ByteBuffer("key_b",5));
                assert(block[0][3].GetKey() == sharpen::ByteBuffer("key_c",5));
                assert(block[0][4].GetKey() == sharpen::ByteBuffer("key_d",5));
                assert(block[1][0].GetKey() == sharpen::ByteBuffer("other",5));
                block.StoreTo(buf);
            }
            {
                sharpen::SstDataBlock block;
                block.LoadFrom(buf);
                assert(block.GetSize() == 2);
                assert(block[0].GetSize() == 5);
                assert(block[1].GetSize() == 1);
                assert(block[0][0].GetKey() == sharpen::ByteBuffer("key_",4));
                assert(block[0][1].GetKey() == sharpen::ByteBuffer("key_a",5));
                assert(block[0][2].GetKey() == sharpen::ByteBuffer("key_b",5));
                assert(block[0][3].GetKey() == sharpen::ByteBuffer("key_c",5));
                assert(block[0][4].GetKey() == sharpen::ByteBuffer("key_d",5));
                assert(block[1][0].GetKey() == sharpen::ByteBuffer("other",5));
                sharpen::SstDataBlock other{block};
                other.Put(sharpen::ByteBuffer{"key_a",5},sharpen::ByteBuffer{"val",3});
                block.Combine(other,false);
                assert(block[sharpen::ByteBuffer("key_a",5)] == sharpen::ByteBuffer("val",3));
                assert(block.IsAtomic());
            }
        }
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
