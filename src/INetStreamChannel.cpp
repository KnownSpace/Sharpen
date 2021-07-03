#include <sharpen/INetStreamChannel.hpp>

#include <sharpen/WinNetStreamChannel.hpp>
#include <sharpen/PosixNetStreamChannel.hpp>
#include <sharpen/AwaitableFuture.hpp>
#include <sharpen/SystemError.hpp>

#ifdef SHARPEN_IS_NIX
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/fcntl.h>
#endif

#ifdef SHARPEN_IS_WIN
#include <WinSock2.h>
#include <mstcpip.h>
#endif

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
    sharpen::FileHandle s = ::socket(afValue,SOCK_STREAM,IPPROTO_TCP);
    if (s == -1)
    {
        sharpen::ThrowLastError();
    }
    int flag;
    flag = ::fcntl(s,F_GETFL,0);
    flag |= O_NONBLOCK;
    ::fcntl(s,F_SETFL,flag);
    channel = std::make_shared<sharpen::PosixNetStreamChannel>(s);
    return std::move(channel);
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

void sharpen::INetStreamChannel::SetKeepAlive(bool val)
{
#ifdef SHARPEN_IS_WIN
	tcp_keepalive keepin;
	tcp_keepalive keepout;
	keepin.keepaliveinterval = 75000;
	keepin.keepalivetime = 7200*1000;
	keepin.onoff = val ? 1:0;
	DWORD ret = 0;
	::WSAIoctl(reinterpret_cast<SOCKET>(this->handle_), SIO_KEEPALIVE_VALS, &keepin, sizeof(keepin), &keepout, sizeof(keepout), &ret, NULL, NULL);
#else
	int opt = val ? 1 : 0;
	int r = ::setsockopt(this->handle_, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));
	if (r == -1)
	{
		sharpen::ThrowLastError();
	}
#endif
}

void sharpen::INetStreamChannel::SetReuseAddress(bool val)
{
#ifdef SHARPEN_IS_WIN
    BOOL opt = val ? TRUE:FALSE;
    ::setsockopt(reinterpret_cast<SOCKET>(this->handle_),SOL_SOCKET,SO_REUSEADDR,reinterpret_cast<char*>(&opt),sizeof(opt));
#else
    int opt = val ? 1:0;
	::setsockopt(this->handle_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
}