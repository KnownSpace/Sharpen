#include <sharpen/IFileChannel.hpp>

#include <sharpen/AwaitableFuture.hpp>
#include <sharpen/EventLoop.hpp>
#include <sharpen/PosixFileChannel.hpp>
#include <sharpen/WinFileChannel.hpp>
#include <sharpen/FileOps.hpp>

#ifdef SHARPEN_IS_NIX
#include <fcntl.h>
#include <unistd.h>
#endif

#include <cassert>

sharpen::FileChannelPtr sharpen::OpenFileChannel(const char *filename,
                                                 sharpen::FileAccessMethod access,
                                                 sharpen::FileOpenMethod open,
                                                 sharpen::FileIoMethod io) {
    sharpen::FileChannelPtr channel;
#ifdef SHARPEN_HAS_WINFILE
    DWORD accessModel = GENERIC_READ;
    DWORD openModel = OPEN_EXISTING;
    // set access and shared
    switch (access) {
    case sharpen::FileAccessMethod::Write:
        accessModel = GENERIC_WRITE;
        break;
    case sharpen::FileAccessMethod::Read:
        accessModel = GENERIC_READ;
        break;
    case sharpen::FileAccessMethod::All:
        accessModel = GENERIC_READ | GENERIC_WRITE;
        break;
    default:
        throw std::logic_error("unkonw access method");
    }
    // set open
    switch (open) {
    case sharpen::FileOpenMethod::Open:
        openModel = OPEN_EXISTING;
        break;
    case sharpen::FileOpenMethod::CreateNew:
        openModel = CREATE_ALWAYS;
        break;
    case sharpen::FileOpenMethod::CreateOrOpen:
        openModel = OPEN_ALWAYS;
        break;
    default:
        std::logic_error("unknow open method");
    }
    DWORD ioFlag{FILE_FLAG_OVERLAPPED};
    bool syncWrite{false};
    switch (io) {
    case sharpen::FileIoMethod::Normal:
        break;
    case sharpen::FileIoMethod::Direct:
        ioFlag |= FILE_FLAG_NO_BUFFERING;
        break;
    case sharpen::FileIoMethod::Sync:
        ioFlag |= FILE_FLAG_WRITE_THROUGH;
        syncWrite = true;
        break;
    case sharpen::FileIoMethod::DirectAndSync:
        ioFlag |= FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH;
        syncWrite = true;
        break;
    default:
        throw std::logic_error("unknow open method");
        break;
    }
#ifndef SHARPEN_NOT_WIN_AUTOSYNC
    // FlushFileBuffers() doesn't have an async version
    // use FILE_FLAG_WRITE_THROUGH for get maximum asynchronous performance
    if (accessModel & GENERIC_WRITE) {
        ioFlag |= FILE_FLAG_WRITE_THROUGH;
        syncWrite = true;
    }
#endif
    // create file
    sharpen::FileHandle handle =
        ::CreateFileA(filename,
                      accessModel,
                      FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                      nullptr,
                      openModel,
                      ioFlag,
                      nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        sharpen::ThrowLastError();
    }
    try {
        channel = std::make_shared<sharpen::WinFileChannel>(handle, syncWrite);
    } catch (const std::exception &rethrow) {
        sharpen::CloseFileHandle(handle);
        throw;
        (void)rethrow;
    }
#else
    std::int32_t accessModel{0};
    std::int32_t openModel{0};
    std::int32_t ioFlag{0};
    // set access and shared
    switch (access) {
    case sharpen::FileAccessMethod::Read:
        accessModel = O_RDONLY;
        break;
    case sharpen::FileAccessMethod::Write:
        accessModel = O_WRONLY;
        break;
    case sharpen::FileAccessMethod::All:
        accessModel = O_RDWR;
        break;
    default:
        throw std::logic_error("unknow access method");
    }
    // set open
    switch (open) {
    case sharpen::FileOpenMethod::Open:
        openModel = 0;
        break;
    case sharpen::FileOpenMethod::CreateNew:
        openModel = O_CREAT | O_TRUNC;
        break;
    case sharpen::FileOpenMethod::CreateOrOpen:
        openModel = O_CREAT;
        break;
    default:
        throw std::logic_error("unknow open method");
    }
    bool syncWrite{false};
    switch (io) {
    case sharpen::FileIoMethod::Normal:
        break;
    case sharpen::FileIoMethod::Direct:
        ioFlag = O_DIRECT;
        break;
    case sharpen::FileIoMethod::Sync:
        ioFlag = O_SYNC;
        syncWrite = true;
        break;
    case sharpen::FileIoMethod::DirectAndSync:
        ioFlag = O_DIRECT | O_SYNC;
        syncWrite = true;
        break;
    default:
        throw std::logic_error("unknow io method");
        break;
    }
    sharpen::FileHandle handle{-1};
    while (handle == -1) {
        handle = ::open(filename, accessModel | openModel | O_CLOEXEC | ioFlag, S_IRUSR | S_IWUSR);
        if (handle == -1) {
            sharpen::ErrorCode error{sharpen::GetLastError()};
            if (error != EINTR) {
                sharpen::ThrowSystemError(error);
            }
        }
    }
    try {
        channel = std::make_shared<sharpen::PosixFileChannel>(handle, syncWrite);
    } catch (const std::exception &rethrow) {
        sharpen::CloseFileHandle(handle);
        throw;
        (void)rethrow;
    }
#endif
    return channel;
}

sharpen::FileChannelPtr sharpen::OpenFileChannel(const char *filename,
                                                 sharpen::FileAccessMethod access,
                                                 sharpen::FileOpenMethod open) {
    return sharpen::OpenFileChannel(filename, access, open, sharpen::FileIoMethod::Normal);
}

void sharpen::IFileChannel::ZeroMemoryAsync(sharpen::Future<std::size_t> &future,
                                            std::size_t size,
                                            std::uint64_t offset) {
    this->WriteAsync("", 1, offset + size - 1, future);
}

std::size_t sharpen::IFileChannel::ZeroMemoryAsync(std::size_t size, std::uint64_t offset) {
    sharpen::AwaitableFuture<std::size_t> future;
    this->ZeroMemoryAsync(future, size, offset);
    return future.Await();
}

void sharpen::IFileChannel::FlushAsync() {
    sharpen::AwaitableFuture<void> future;
    this->FlushAsync(future);
    return future.Await();
}

std::size_t sharpen::IFileChannel::AllocateAsync(std::uint64_t offset, std::size_t size) {
    sharpen::AwaitableFuture<std::size_t> future;
    this->AllocateAsync(future,offset,size);
    return future.Await();
}

std::size_t sharpen::IFileChannel::DeallocateAsync(std::uint64_t offset, std::size_t size) {
    sharpen::AwaitableFuture<std::size_t> future;
    this->DeallocateAsync(future,offset,size);
    return future.Await();
}

void sharpen::IFileChannel::Remove() {
    thread_local char path[sharpen::GetMaxPath() + 1] = {0};
    std::memset(path,0,sizeof(path));
    std::size_t size{this->GetPath(path,sizeof(path))};
    (void)size;
    this->Close();
    sharpen::RemoveFile(path);
}