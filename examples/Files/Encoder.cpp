#include <cstdio>
#include <sharpen/IFileChannel.hpp>
#include <sharpen/EventEngine.hpp>
#include <sharpen/FileOps.hpp>

void Entry(const char *src,const char *dst)
{
    if(!sharpen::ExistFile(src) || !sharpen::AccessFile(src,sharpen::FileAccessModel::Read))
    {
        std::puts("cannot open source file");
    }
    sharpen::FileChannelPtr chann = sharpen::MakeFileChannel(src,sharpen::FileAccessModel::Read,sharpen::FileOpenModel::Open);
    chann->Register(sharpen::EventEngine::GetEngine());
    sharpen::ByteBuffer buf{chann->GetFileSize()};
    chann->ReadAsync(buf,0);
    for (auto begin = buf.Begin(),end = buf.End(); begin != end; ++begin)
    {
        *begin ^= 0x64;
    }
    chann = sharpen::MakeFileChannel(dst,sharpen::FileAccessModel::Write,sharpen::FileOpenModel::CreateNew);
    chann->Register(sharpen::EventEngine::GetEngine());
    chann->WriteAsync(buf,0);
}

int main(int argc, char const *argv[])
{
    if(argc < 3)
    {
        std::puts("usage: <src path> <dst path>");
    }
    sharpen::EventEngine &engine = sharpen::EventEngine::SetupSingleThreadEngine();
    engine.Startup(&Entry,argv[1],argv[2]);
    return 0;
}
