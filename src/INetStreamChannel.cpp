#include <sharpen/INetStreamChannel.hpp>

#include <sharpen/WinNetStreamChannel.hpp>
#include <sharpen/PosixNetStreamChannel.hpp>
#include <sharpen/AwaitableFuture.hpp>
#include <sharpen/SystemError.hpp>

#ifdef SHARPEN_IS_NIX
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/fcntl.h>
#include <signal.h>
#endif

#ifdef SHARPEN_IS_WIN
#include <WinSock2.h>
#include <mstcpip.h>
#endif

sharpen::NetStreamChannelPtr sharpen::OpenTcpChannel(sharpen::AddressFamily af)
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
    try
    {
        channel = std::make_shared<sharpen::WinNetStreamChannel>(reinterpret_cast<sharpen::FileHandle>(s),afValue);
    }
    catch(const std::exception& rethrow)
    {
        ::closesocket(s);
        throw;
        (void)rethrow;
    }
#else
    sharpen::FileHandle s = ::socket(afValue,SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK,IPPROTO_TCP);
    if (s == -1)
    {
        sharpen::ThrowLastError();
    }
    try
    {
        channel = std::make_shared<sharpen::PosixNetStreamChannel>(s);
    }
    catch(const std::exception& rethrow)
    {
        sharpen::CloseFileHandle(s);
        throw;
        (void)rethrow;
    }
#endif
    return channel;
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
#else
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE,&sa,0);
#endif
}

void sharpen::CleanupNetSupport()
{
#ifdef SHARPEN_HAS_WINSOCKET
    WSACleanup();
#else
    struct sigaction sa;
    sa.sa_handler = SIG_DFL;
    sigaction(SIGPIPE,&sa,0);
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

void sharpen::INetStreamChannel::SendFileAsync(sharpen::FileChannelPtr file,std::uint64_t size,std::uint64_t offset)
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

void sharpen::INetStreamChannel::Listen(std::uint16_t queueLength)
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

int sharpen::INetStreamChannel::GetErrorCode() const noexcept
{
    int err{0};
#ifdef SHARPEN_IS_WIN
    int errSize = sizeof(err);
    ::getsockopt(reinterpret_cast<SOCKET>(this->handle_),SOL_SOCKET,SO_ERROR,reinterpret_cast<char*>(&err),&errSize);
#else
    socklen_t errSize = sizeof(err);
    ::getsockopt(this->handle_,SOL_SOCKET,SO_ERROR,&err,&errSize);
#endif
    return err;
}

void sharpen::INetStreamChannel::PollReadAsync()
{
    sharpen::AwaitableFuture<void> future;
    this->PollReadAsync(future);
    future.Await();
}

void sharpen::INetStreamChannel::PollWriteAsync()
{
    sharpen::AwaitableFuture<void> future;
    this->PollWriteAsync(future);
    future.Await();
}