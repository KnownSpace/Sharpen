#pragma once
#ifndef _SHARPEN_BUFFEROPS_HPP
#define _SHARPEN_BUFFEROPS_HPP

#include "TypeTraits.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace sharpen {
    extern unsigned char Crc16TableHeight[256];

    extern unsigned char Crc16TableLow[256];

    extern std::uint32_t Crc32Table[256];

    // CRC16-MODBUS
    extern std::uint16_t Crc16(const char *data, std::size_t size) noexcept;

    // CRC32
    extern std::uint32_t Crc32(const char *data, std::size_t size) noexcept;

    // Adler32
    extern std::uint32_t Adler32(const char *data, std::size_t size) noexcept;

    // Base64 Encode Size
    extern std::size_t ComputeBase64EncodeSize(std::size_t size) noexcept;

    // Base64 Decode Size
    extern std::size_t ComputeBase64DecodeSize(std::size_t size) noexcept;

    // Base64 Encode
    extern bool Base64Encode(char *dst,
                             std::size_t dstSize,
                             const char *src,
                             std::size_t srcSize) noexcept;

    // Base64 Decode Mapping
    extern char Base64DecodeMapping(unsigned char c) noexcept;

    // Base64 Decode
    extern bool Base64Decode(char *dst,
                             std::size_t dstSize,
                             const char *src,
                             std::size_t srcSize) noexcept;

    // FNV-1a hash 32bits
    template<typename _Iterator,
             typename _Check = decltype(static_cast<char>(0) == *std::declval<_Iterator &>()++)>
    inline std::uint32_t BufferHash32(_Iterator begin, _Iterator end) noexcept {
        constexpr std::uint32_t offsetBasis = 0x811c9dc5;
        constexpr std::uint32_t prime = 0x01000193;
        std::uint32_t hash = offsetBasis;
        while (begin != end) {
            hash ^= *begin;
            hash *= prime;
            ++begin;
        }
        return hash;
    }

    // FNV-1a hash 32bits
    extern std::uint32_t BufferHash32(const char *data, std::size_t size) noexcept;

    // FNV-1a hash 64bits
    template<typename _Iterator,
             typename _Check = decltype(static_cast<char>(0) == *std::declval<_Iterator &>()++)>
    inline std::uint64_t BufferHash64(_Iterator begin, _Iterator end) noexcept {
        constexpr std::uint64_t offsetBasis = 0xcbf29ce484222325;
        constexpr std::uint64_t prime = 0x00000100000001B3;
        std::uint64_t hash{offsetBasis};
        while (begin != end) {
            hash ^= *begin;
            hash *= prime;
            ++begin;
        }
        return hash;
    }

    // FNV-1a hash 64bits
    extern std::uint64_t BufferHash64(const char *data, std::size_t size) noexcept;

    template<typename _U,
             typename _Iterator,
             typename _Check = sharpen::EnableIf<std::is_same<std::size_t, std::uint64_t>::value>,
             typename _CheckIterator = decltype(static_cast<char>(0) ==
                                                *std::declval<_Iterator &>()++)>
    inline _U InternalBetterBufferHash(_Iterator begin, _Iterator end, int) noexcept {
        return BufferHash64(begin, end);
    }

    template<typename _U,
             typename _Iterator,
             typename _Check = decltype(static_cast<char>(0) == *std::declval<_Iterator &>()++)>
    inline _U InternalBetterBufferHash(_Iterator begin, _Iterator end, ...) noexcept {
        return BufferHash32(begin, end);
    }

    inline std::size_t BufferHash(const char *data, std::size_t size) noexcept {
#ifndef SHARPEN_FORCE_HASH32
        return sharpen::InternalBetterBufferHash<std::size_t>(data, data + size, 0);
#else
        return sharpen::BufferHash32(data, size);
#endif
    }

    template<typename _Iterator,
             typename _Check = decltype(static_cast<char>(0) == *std::declval<_Iterator>())>
    inline std::size_t BufferHash(_Iterator begin, _Iterator end) {
#ifndef SHARPEN_FORCE_HASH32
        return sharpen::InternalBetterBufferHash<std::size_t>(begin, end, 0);
#else
        return sharpen::BufferHash32(begin, end);
#endif
    }

    // 1     if leftBuf >  rightBuf
    // 0     if leftBuf == rightBuf
    //-1    if leftBuf <  rigthBuf
    extern std::int32_t BufferCompare(const char *leftBuf,
                                      std::size_t leftSize,
                                      const char *rightBuf,
                                      std::size_t rightSize) noexcept;
}   // namespace sharpen

#endif