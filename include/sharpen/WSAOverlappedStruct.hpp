#pragma once
#ifndef _SHARPEN_WSAOVERLAPPEDSTRUCT_HPP
#define _SHARPEN_WSAOVERLAPPEDSTRUCT_HPP

#include "IocpOverlappedStruct.hpp"

#ifdef SHARPEN_HAS_IOCP

#include <ws2def.h>

namespace sharpen
{
    struct WSAOverlappedStruct : public sharpen::IocpOverlappedStruct
    {
        WSABUF buf_;
        sharpen::FileHandle accepted_;
    };
}   // namespace sharpen
#endif

#endif