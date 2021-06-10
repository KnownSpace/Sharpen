#include <sharpen/INetStreamChannel.hpp>
#include <sharpen/WinNetStreamChannel.hpp>
#include <sharpen/PosixNetStreamChannel.hpp>

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