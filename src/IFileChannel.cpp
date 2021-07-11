#include <sharpen/WinFileChannel.hpp>
#include <sharpen/PosixFileChannel.hpp>

#include <cassert>

#include <sharpen/EventLoop.hpp>

#ifdef SHARPEN_IS_NIX
#include <unistd.h>
#include <fcntl.h>
#endif

sharpen::FileChannelPtr sharpen::MakeFileChannel(const char *filename,sharpen::FileAccessModel access,sharpen::FileOpenModel open)
{
    sharpen::FileChannelPtr channel;
#ifdef SHARPEN_HAS_WINFILE
    DWORD sharedModel,accessModel,openModel;
    //set access and shared
    switch (access)
    {
    case sharpen::FileAccessModel::Write:
        accessModel = FILE_GENERIC_WRITE;
        sharedModel = FILE_SHARE_WRITE;
        break;
    case sharpen::FileAccessModel::Read:
        accessModel = FILE_GENERIC_READ;
        sharedModel = FILE_SHARE_READ;
        break;
    case sharpen::FileAccessModel::All:
        accessModel = FILE_ALL_ACCESS;
        sharedModel = FILE_SHARE_READ | FILE_SHARE_WRITE;
        break;
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
    }
    //create file
    sharpen::FileHandle handle = ::CreateFileA(filename,accessModel,sharedModel,nullptr,openModel,FILE_FLAG_OVERLAPPED,INVALID_HANDLE_VALUE);
    if (handle == INVALID_HANDLE_VALUE)
    {
        sharpen::ThrowLastError();
    }
    channel = std::make_shared<sharpen::WinFileChannel>(handle);
#else
    sharpen::Int32 accessModel,openModel;
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
    }
    sharpen::FileHandle handle = ::open(filename,accessModel | openModel);
    if (handle == -1)
    {
        sharpen::ThrowLastError();
    }
    channel = std::make_shared<sharpen::PosixFileChannel>(handle);
#endif
    return std::move(channel);
}