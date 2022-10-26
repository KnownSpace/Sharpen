#include <sharpen/WinFileChannel.hpp>
#include <sharpen/PosixFileChannel.hpp>

#include <cassert>

#include <sharpen/EventLoop.hpp>
#include <sharpen/AwaitableFuture.hpp>

#ifdef SHARPEN_IS_NIX
#include <unistd.h>
#include <fcntl.h>
#endif

sharpen::FileChannelPtr sharpen::MakeFileChannel(const char *filename,sharpen::FileAccessModel access,sharpen::FileOpenModel open,bool direct)
{
    sharpen::FileChannelPtr channel;
#ifdef SHARPEN_HAS_WINFILE
    DWORD accessModel = FILE_GENERIC_READ;
    DWORD openModel = OPEN_EXISTING;
    //set access and shared
    switch (access)
    {
    case sharpen::FileAccessModel::Write:
        accessModel = FILE_GENERIC_WRITE;
        break;
    case sharpen::FileAccessModel::Read:
        accessModel = FILE_GENERIC_READ;
        break;
    case sharpen::FileAccessModel::All:
        accessModel = FILE_GENERIC_READ | FILE_GENERIC_WRITE;
        break;
    default:
        throw std::logic_error("unkonw access model");
    }
    //set open
    switch (open)
    {
    case sharpen::FileOpenModel::Open:
        openModel = OPEN_EXISTING;
        break;
    case sharpen::FileOpenModel::CreateNew:
        openModel = CREATE_ALWAYS;
        break;
    case sharpen::FileOpenModel::CreateOrOpen:
        openModel = OPEN_ALWAYS;
        break;
    default:
        std::logic_error("unknow open model");
    }
    DWORD flag{FILE_FLAG_OVERLAPPED};
    if(direct)
    {
        flag |= FILE_FLAG_NO_BUFFERING;
        flag |= FILE_FLAG_WRITE_THROUGH;
    }
    //create file
    sharpen::FileHandle handle = ::CreateFileA(filename,accessModel,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,nullptr,openModel,flag,INVALID_HANDLE_VALUE);
    if (handle == INVALID_HANDLE_VALUE)
    {
        sharpen::ThrowLastError();
    }
    channel = std::make_shared<sharpen::WinFileChannel>(handle);
#else
    std::int32_t accessModel{0};
    std::int32_t openModel{0};
    std::int32_t flag{0};
    //set access and shared
    switch (access)
    {
    case sharpen::FileAccessModel::Read:
        accessModel = O_RDONLY;
        break;
    case sharpen::FileAccessModel::Write:
        accessModel = O_WRONLY;
        break;
    case sharpen::FileAccessModel::All:
        accessModel = O_RDWR;
        break;
    default:
        throw std::logic_error("unknow access model");
    }
    //set open
    switch (open)
    {
    case sharpen::FileOpenModel::Open:
        openModel = 0;
        break;
    case sharpen::FileOpenModel::CreateNew:
        openModel = O_CREAT | O_TRUNC;
        break;
    case sharpen::FileOpenModel::CreateOrOpen:
        openModel = O_CREAT;
        break;
    default:
        throw std::logic_error("unknow open model");
    }
    if(direct)
    {
        flag = O_DIRECT;
    }
    sharpen::FileHandle handle = ::open(filename,accessModel | openModel | O_CLOEXEC | flag,S_IRUSR|S_IWUSR);
    if (handle == -1)
    {
        sharpen::ThrowLastError();
    }
    channel = std::make_shared<sharpen::PosixFileChannel>(handle);
#endif
    return channel;
}

sharpen::FileChannelPtr sharpen::MakeFileChannel(const char *filename,sharpen::FileAccessModel access,sharpen::FileOpenModel open)
{
    return sharpen::MakeFileChannel(filename,access,open,false);
}

void sharpen::IFileChannel::ZeroMemoryAsync(sharpen::Future<std::size_t> &future,std::size_t size,std::uint64_t offset)
{
    this->WriteAsync("",1,offset + size - 1,future);
}

std::size_t sharpen::IFileChannel::ZeroMemoryAsync(std::size_t size,std::uint64_t offset)
{
    sharpen::AwaitableFuture<std::size_t> future;
    this->ZeroMemoryAsync(future,size,offset);
    return future.Await();
}