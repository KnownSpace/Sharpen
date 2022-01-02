#pragma once
#ifndef _SHARPEN_BUFFEROPS_HPP
#define _SHARPEN_BUFFEROPS_HPP

#include "TypeDef.hpp"

namespace sharpen
{
    extern unsigned char Crc16TableHeight[256];

    extern unsigned char Crc16TableLow[256];

    //CRC16-MODBUS
    sharpen::Uint16 Crc16(const char *data,sharpen::Size size) noexcept;

    //Adler32
    sharpen::Uint32 Adler32(const char *data,sharpen::Size size) noexcept;   

    //Base64 Encode Size
    sharpen::Size ComputeBase64EncodeSize(sharpen::Size size) noexcept;

    //Base64 Decode Size
    sharpen::Size ComputeBase64DecodeSize(sharpen::Size size) noexcept;

    //Base64 Encode
    bool Base64Encode(char *dst,sharpen::Size dstSize,const char *src,sharpen::Size srcSize) noexcept;

    //Base64 Decode Mapping
    char Base64DecodeMapping(unsigned char c) noexcept;

    //Base64 Decode
    bool Base64Decode(char *dst,sharpen::Size dstSize,const char *src,sharpen::Size srcSize) noexcept;

    //FNV-1a hash 32bits
    sharpen::Size BufferHash(const char *data,sharpen::Size size) noexcept;
}

#endif