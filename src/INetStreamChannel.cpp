#include <sharpen/INetStreamChannel.hpp>

#include <sharpen/WinNetStreamChannel.hpp>
#include <sharpen/PosixNetStreamChannel.hpp>
#include <sharpen/AwaitableFuture.hpp>

sharpen::NetStreamChannelPtr sharpen::MakeTcpStreamChannel(sharpen::AddressFamily af)
{
    sharpen::NetStreamChannelPtr channel;
    int afValue;
    if (af == sharpen::AddressFamily::Ip)
    {
        afValue = AF_INET;
    }
    else
    {
        afValue = AF_INET6;
    }
#ifdef SHARPEN_HAS_WINSOCKET
    SOCKET s = ::socket(afValue,SOCK_STREAM,IPPROTO_TCP);
    if (s == INVALID_SOCKET)
    {
        sharpen::ThrowLastError();
    }
    channel = std::make_shared<sharpen::WinNetStreamChannel>(reinterpret_cast<sharpen::FileHandle>(s),afValue);
    return std::move(channel);
#else

#endif
}

void sharpen::StartupNetSupport()
{
#ifdef SHARPEN_HAS_WINSOCKET
    WORD version = MAKEWORD(2,2);
    WSADATA data;
    int r = WSAStartup(version,&data);
    if (r != 0)
    {
        sharpen::ThrowLastError();
    }
#endif
}

void sharpen::CleanupNetSupport()
{
#ifdef SHARPEN_HAS_WINSOCKET
    WSACleanup();
#endif
}

void sharpen::INetStreamChannel::ConnectAsync(const sharpen::IEndPoint &endpoint)
{
    sharpen::AwaitableFuture<void> future;
    this->ConnectAsync(endpoint,future);
    future.Await();
}

sharpen::NetStreamChannelPtr sharpen::INetStreamChannel::AcceptAsync()
{
    sharpen::AwaitableFuture<sharpen::NetStreamChannelPtr> future;
    this->AcceptAsync(future);
    return future.Await();
}

void sharpen::INetStreamChannel::SendFileAsync(sharpen::FileChannelPtr file,sharpen::Uint64 size,sharpen::Uint64 offset)
{
    sharpen::AwaitableFuture<void> future;
    this->SendFileAsync(file,size,offset,future);
    future.Await();
}

void sharpen::INetStreamChannel::SendFileAsync(sharpen::FileChannelPtr file)
{
    sharpen::AwaitableFuture<void> future;
    this->SendFileAsync(file,future);
    future.Await();
}

void sharpen::INetStreamChannel::Bind(const sharpen::IEndPoint &endpoint)
{
#ifdef SHARPEN_HAS_WINSOCKET
    int r = ::bind(reinterpret_cast<SOCKET>(this->handle_),endpoint.GetAddrPtr(),endpoint.GetAddrLen());
#else
    int r = ::bind(this->handle_,endpoint.GetAddrPtr(),endpoint.GetAddrLen());
#endif
    if (r != 0)
    {
        sharpen::ThrowLastError();
    }
}

void sharpen::INetStreamChannel::Listen(sharpen::Uint16 queueLength)
{
#ifdef SHARPEN_HAS_WINSOCKET
    int r = ::listen(reinterpret_cast<SOCKET>(this->handle_),queueLength);
#else
    int r = ::listen(this->handle_,queueLength);
#endif
    if (r != 0)
    {
        sharpen::ThrowLastError();
    }
}

void sharpen::INetStreamChannel::GetLocalEndPoint(sharpen::IEndPoint &endPoint) const
{
#ifdef SHARPEN_HAS_WINSOCKET
    int len = endPoint.GetAddrLen();
    int r = ::getsockname(reinterpret_cast<SOCKET>(this->handle_),endPoint.GetAddrPtr(),&len);
#else
    socklen_t len = endPoint.GetAddrLen();
    int r = ::getsockname(this->handle_,endPoint.GetAddrPtr(),&len);
#endif
    if (r != 0)
    {
        sharpen::ThrowLastError();
    }
    
}

void sharpen::INetStreamChannel::GetRemoteEndPoint(sharpen::IEndPoint &endPoint) const
{
#ifdef SHARPEN_HAS_WINSOCKET
    int len = endPoint.GetAddrLen();
    int r = ::getpeername(reinterpret_cast<SOCKET>(this->handle_),endPoint.GetAddrPtr(),&len);
#else
    socklen_t len = endPoint.GetAddrLen();
    int r = ::getpeername(this->handle_,endPoint.GetAddrPtr(),&len);
#endif
    if (r != 0)
    {
        sharpen::ThrowLastError();
    }
}