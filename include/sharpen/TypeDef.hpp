#pragma once
#ifndef _SHARPEN_TYPEDEF_HPP
#define _SHARPEN_TYPEDEF_HPP

#include <cstdint>
#include <cstddef>
namespace sharpen
{
    //int types
    using Int16 = std::int16_t;
    using Int32 = std::int32_t;
    using Int64 = std::int64_t;
    using Intptr = std::intptr_t;

    //uint types
    using Uint16 = std::uint16_t;
    using Uint32 = std::uint32_t;
    using Uint64 = std::uint64_t;
    using Uintptr = std::uintptr_t;

    //byte types
    using Char = char;
    using Byte = unsigned char;

    //other types
    using Size = std::size_t;
    using UintPort = sharpen::Uint16;
    using UintIpAddr = sharpen::Uint32;
    using UintIpv6Addr = sharpen::Uint64;
}

#endif
