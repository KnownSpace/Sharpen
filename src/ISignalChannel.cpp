#include <sharpen/ISignalChannel.hpp>


#ifdef SHARPEN_IS_WIN
#include <csignal>

#include <Windows.h>

#include <sharpen/WinEx.h>
#else
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#endif

#include <sharpen/AwaitableFuture.hpp>
#include <sharpen/SystemError.hpp>

#ifdef SHARPEN_USE_SIGNALFD
#include <sharpen/LinuxSignalFdChannel.hpp>
#else
#include <sharpen/PosixPipeSignalChannel.hpp>
#include <sharpen/WinPipeSignalChannel.hpp>
#endif

#ifdef SHARPEN_USE_PIPESIGNAL
std::unique_ptr<sharpen::SignalMap> sharpen::SignalStorage::sigmap;

std::once_flag sharpen::SignalStorage::sigmapFlag;

std::atomic_uint64_t sharpen::SignalStorage::sigBitset;

void sharpen::SignalStorage::DoInstallHandler()
{
    sharpen::SignalMap *map{new (std::nothrow) sharpen::SignalMap{}};
    if (!map)
    {
        throw std::bad_alloc{};
    }
    sigmap.reset(map);
}

void SHARPEN_SignalHandler(int sig)
{
    if (sharpen::SignalStorage::sigmap)
    {
        sharpen::SignalStorage::sigmap->Raise(sig);
    }
#ifdef SHARPEN_IS_WIN
    // re-install handler
    std::signal(sig, &SHARPEN_SignalHandler);
#endif
}

void sharpen::SignalStorage::InstallHandler(std::int32_t sig)
{
    using FnPtr = void (*)();
    std::call_once(sigmapFlag, static_cast<FnPtr>(Self::DoInstallHandler));
    assert(sig > 0);
    std::uint32_t sigBitPos{static_cast<std::uint32_t>(sig - 1)};
    std::uint64_t sigBit{static_cast<std::uint64_t>(1) << sigBitPos};
    std::uint64_t bitset{sigBitset.load()};
    if (bitset & sigBit)
    {
        std::uint64_t newBitset{0};
        do {
            newBitset = bitset | sigBit;
        } while (!sigBitset.compare_exchange_weak(bitset, newBitset));
    }
    if (!(bitset & sigBit))
    {
        // install handler
#ifdef SHARPEN_IS_WIN
        // use signal, we need re-install handler
        std::signal(sig, &SHARPEN_SignalHandler);
#else
        // use sigaction
        struct sigaction sigact;
        sigact.sa_handler = &SHARPEN_SignalHandler;
        if (::sigaction(sig, &sigact, nullptr) == -1)
        {
            std::terminate();
        }
#endif
    }
}
#endif

std::size_t sharpen::ISignalChannel::ReadAsync(sharpen::SignalBuffer &signals)
{
    sharpen::AwaitableFuture<std::size_t> future;
    this->ReadAsync(signals, future);
    return future.Await();
}

sharpen::SignalChannelPtr sharpen::OpenSignalChannel(std::int32_t *sigs, std::size_t sigCount)
{
#ifdef SHARPEN_IS_WIN
    sharpen::FileHandle reader{INVALID_HANDLE_VALUE};
    sharpen::FileHandle writer{INVALID_HANDLE_VALUE};
    BOOL r = ::CreatePipeEx(&reader,
                            &writer,
                            nullptr,
                            static_cast<DWORD>(sigCount * sizeof(*sigs)),
                            FILE_FLAG_OVERLAPPED,
                            0);
    if (r == FALSE)
    {
        sharpen::ThrowLastError();
    }
    for (std::size_t i = 0; i != sigCount; ++i)
    {
        sharpen::SignalStorage::InstallHandler(sigs[i]);
        sharpen::SignalStorage::sigmap->Register(writer, sigs[i]);
    }
    sharpen::SignalChannelPtr channel{nullptr};
    try
    {
        channel = std::make_shared<sharpen::WinPipeSignalChannel>(
            reader, writer, *sharpen::SignalStorage::sigmap);
    }
    catch (const std::exception &rethrow)
    {
        sharpen::SignalStorage::sigmap->Unregister(writer);
        sharpen::CloseFileHandle(writer);
        sharpen::CloseFileHandle(reader);
        (void)rethrow;
        throw;
    }
    return channel;
#elif (defined(SHARPEN_USE_PIPESIGNAL))
    sharpen::FileHandle pipes[2];
    if (::pipe2(pipes, O_NONBLOCK | O_CLOEXEC) == -1)
    {
        sharpen::ThrowLastError();
    }
    sharpen::FileHandle writer{pipes[1]};
    sharpen::FileHandle reader{pipes[0]};
    for (std::size_t i = 0; i != sigCount; ++i)
    {
        sharpen::SignalStorage::InstallHandler(sigs[i]);
        sharpen::SignalStorage::sigmap->Register(writer, sigs[i]);
    }
    sharpen::SignalChannelPtr channel{nullptr};
    try
    {
        channel = std::make_shared<sharpen::PosixPipeSignalChannel>(
            reader, writer, *sharpen::SignalStorage::sigmap);
    }
    catch (const std::exception &rethrow)
    {
        sharpen::SignalStorage::sigmap->Unregister(writer);
        sharpen::CloseFileHandle(writer);
        sharpen::CloseFileHandle(reader);
        (void)rethrow;
        throw;
    }
    return channel;
#else
    sigset_t sigset;
    ::sigemptyset(&sigset);
    for (std::size_t i = 0; i != sigCount; ++i)
    {
        ::sigaddset(&sigset, sigs[i]);
    }
    ::sigprocmask(SIG_BLOCK, &sigset, nullptr);
    sharpen::FileHandle signalFd{sharpen::OpenSignalFd(sigset)};
    if (signalFd == -1)
    {
        sharpen::ThrowLastError();
    }
    sharpen::SignalChannelPtr channel{nullptr};
    try
    {
        channel = std::make_shared<sharpen::LinuxSignalFdChannel>(signalFd);
    }
    catch (const std::exception &rethrow)
    {
        sharpen::CloseFileHandle(signalFd);
        throw;
        (void)rethrow;
    }
    return channel;
#endif
}