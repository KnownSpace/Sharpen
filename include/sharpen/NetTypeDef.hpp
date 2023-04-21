#pragma once
#ifndef _SHARPEN_NETTYPEDEF_HPP
#define _SHARPEN_NETTYPEDEF_HPP

namespace sharpen
{
    enum class AddressFamily
    {
        Ip,
        Ipv6
    };

    enum class SocketType
    {
        Stream,
        Dgram,
        Raw
    };

    enum class NetProtocol
    {
        Tcp,
        Udp
    };
}   // namespace sharpen

#endif