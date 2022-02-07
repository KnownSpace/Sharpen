#pragma once
#ifndef _SHARPEN_BUFFEROPS_HPP
#define _SHARPEN_BUFFEROPS_HPP

#include "TypeDef.hpp"
#include "TypeTraits.hpp"

#include <cstring>

namespace sharpen
{
    extern unsigned char Crc16TableHeight[256];

    extern unsigned char Crc16TableLow[256];

    //CRC16-MODBUS
    sharpen::Uint16 Crc16(const char *data, sharpen::Size size) noexcept;

    //Adler32
    sharpen::Uint32 Adler32(const char *data, sharpen::Size size) noexcept;

    //Base64 Encode Size
    sharpen::Size ComputeBase64EncodeSize(sharpen::Size size) noexcept;

    //Base64 Decode Size
    sharpen::Size ComputeBase64DecodeSize(sharpen::Size size) noexcept;

    //Base64 Encode
    bool Base64Encode(char *dst, sharpen::Size dstSize, const char *src, sharpen::Size srcSize) noexcept;

    //Base64 Decode Mapping
    char Base64DecodeMapping(unsigned char c) noexcept;

    //Base64 Decode
    bool Base64Decode(char *dst, sharpen::Size dstSize, const char *src, sharpen::Size srcSize) noexcept;

    //FNV-1a hash 32bits
    template<typename _Iterator,typename _Check = decltype(static_cast<char>(0) == *std::declval<_Iterator>())>
    inline sharpen::Size BufferHash32(_Iterator begin,_Iterator end) noexcept
    {
        constexpr sharpen::Size offsetBasis = 0x811c9dc5;
        constexpr sharpen::Size prime = 0x01000193;
        sharpen::Size hash = offsetBasis;
        while(begin != end)
        {
            hash ^= *begin;
            hash *= prime;
            ++begin;
        }
        return hash;
    }

    //FNV-1a hash 32bits
    sharpen::Size BufferHash32(const char *data, sharpen::Size size) noexcept;

    //FNV-1a hash 64bits
    template<typename _Iterator,typename _Check = decltype(static_cast<char>(0) == *std::declval<_Iterator>())>
    inline sharpen::Uint64 BufferHash64(_Iterator begin,_Iterator end) noexcept
    {
        constexpr sharpen::Uint64 offsetBasis = 0xcbf29ce484222325;
        constexpr sharpen::Uint64 prime = 0x00000100000001B3;
        sharpen::Size hash = offsetBasis;
        while(begin != end)
        {
            hash ^= *begin;
            hash *= prime;
            ++begin;
        }
        return hash;
    }

    //FNV-1a hash 64bits
    sharpen::Uint64 BufferHash64(const char *data, sharpen::Size size) noexcept;

    template <typename _U, typename _Check = sharpen::EnableIf<std::is_same<sharpen::Size, sharpen::Uint64>::value>>
    inline _U InternalBetterBufferHash(const char *data,sharpen::Size size,int) noexcept
    {
        return BufferHash64(data,size);
    }

    template <typename _U>
    inline _U InternalBetterBufferHash(const char *data,sharpen::Size size,...) noexcept
    {
        return BufferHash32(data,size);
    }

    inline sharpen::Size BufferHash(const char *data,sharpen::Size size) noexcept
    {
#ifndef SHARPEN_FORCE_HASH32
        return sharpen::InternalBetterBufferHash<sharpen::Size>(data,size,0);
#else
        return sharpen::BufferHash32(data,size);
#endif
    }

    //1     if leftBuf >  rightBuf
    //0     if leftBuf == rightBuf
    //-1    if leftBuf <  rigthBuf
    sharpen::Int32 BufferCompare(const char *leftBuf,sharpen::Size leftSize,const char *rightBuf,sharpen::Size rightSize) noexcept;
}

#endif