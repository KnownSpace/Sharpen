#include <sharpen/BufferOps.hpp>

#include <utility>
#include <cassert>

#include <sharpen/IntOps.hpp>

unsigned char sharpen::Crc16TableHeight[256] =
{
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,

    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,

    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,

    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,

    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,

    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,

    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,

    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,

    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,

    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,

    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,

    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,

    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,

    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,

    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,

    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,

    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,

    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,

    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,

    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,

    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,

    0x00, 0xC1, 0x81, 0x40
};

unsigned char sharpen::Crc16TableLow[256] =
{

    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7,

    0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,

    0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9,

    0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,

    0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,

    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,

    0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D,

    0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,

    0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF,

    0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,

    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1,

    0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,

    0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB,

    0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,

    0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,

    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,

    0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97,

    0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,

    0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89,

    0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,

    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83,

    0x41, 0x81, 0x80, 0x40
};

std::uint16_t sharpen::Crc16(const char *data, std::size_t size) noexcept
{
    sharpen::UintUnion<16> crc;
    crc.value_ = 0xFFFF;
    unsigned char index{0};
    for (const char *end = data + size; data != end; ++data)
    {
        index = crc.union_.height_ ^ *data;
        crc.union_.height_ = crc.union_.low_ ^ Crc16TableHeight[index];
        crc.union_.low_ = Crc16TableLow[index];
    }
    std::swap(crc.union_.low_,crc.union_.height_);
    return crc.value_;
}

std::uint32_t sharpen::Adler32(const char *data,std::size_t size) noexcept
{
    sharpen::UintUnion<32> adler32{};
    adler32.union_.low_ = 1;
    for (const char *end = data + size; data != end; ++data)
    {
        adler32.union_.low_ += *data;
        adler32.union_.low_ %= 65521;
        adler32.union_.height_ += adler32.union_.low_;
        adler32.union_.height_ %= 65521;
    }
    return adler32.value_;
}

std::size_t sharpen::ComputeBase64EncodeSize(std::size_t size) noexcept
{
    return size/3*4 + ((size % 3) ? 4:0);
}

std::size_t sharpen::ComputeBase64DecodeSize(std::size_t size) noexcept
{
    assert(size % 4 == 0);
    return size/4*3;
}

bool sharpen::Base64Encode(char *dst,std::size_t dstSize,const char *src,std::size_t srcSize) noexcept
{
    if(dstSize < sharpen::ComputeBase64EncodeSize(srcSize))
    {
        return false;
    }
    if(!srcSize)
    {
        return true;
    }
    static const char *base64Hex = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    unsigned char srcBuf[3] = {};
    const char *srcEnd = src + srcSize;
    std::size_t i{0};
    while (src != srcEnd)
    {
        srcBuf[i++] = static_cast<unsigned char>(*(src++));
        if(i == 3)
        {
            //convert to base 64
            *(dst++) = base64Hex[srcBuf[0] >> 2];
            *(dst++) = base64Hex[(srcBuf[0] & 0x03 ) << 4 | srcBuf[1] >> 4];
            *(dst++) = base64Hex[(srcBuf[1] & 0x0F) << 2 | srcBuf[2] >> 6];
            *(dst++) = base64Hex[srcBuf[2] & 0x3F];
            i = 0;
            continue;
        }
    }
    if(i != 0)
    {
        for (std::size_t j = i; j != 3; ++j)
        {
            srcBuf[j] = 0;
        }
        *(dst++) = base64Hex[srcBuf[0] >> 2];
        *(dst++) = base64Hex[(srcBuf[0] & 0x03 ) << 4 | srcBuf[1] >> 4];
        *(dst++) = i == 2 ? base64Hex[(srcBuf[1] & 0x0F) << 2 | srcBuf[2] >> 6] : '=';
        *(dst++) = '=';
    }
    return true;
}

char sharpen::Base64DecodeMapping(unsigned char c) noexcept
{
    if(c >= 'A' && c <= 'Z')
    {
        return c - 'A';
    }
    else if(c >= 'a' && c <= 'z')
    {
        return c - 'a' + 26;
    }
    else if(c == '+')
    {
        return 62;
    }
    else if(c == '=')
    {
        return 0;
    }
    else if(c >= '0' && c <= '9')
    {
        return c - '0' + 52;
    }
    return 63;
}

bool sharpen::Base64Decode(char *dst,std::size_t dstSize,const char *src,std::size_t srcSize) noexcept
{
    if (dstSize < sharpen::ComputeBase64DecodeSize(srcSize) || srcSize % 4)
    {
        return false;
    }
    if(!srcSize)
    {
        return true;
    }
    unsigned char srcBuf[4] = {};
    const char *srcEnd = src + srcSize;
    std::size_t i{0};
    while (src != srcEnd)
    {
        srcBuf[i++] = static_cast<unsigned char>(*(src++));
        if(i == 4)
        {
            char tmp{sharpen::Base64DecodeMapping(srcBuf[0])};
            tmp <<= 2;
            tmp |= sharpen::Base64DecodeMapping(srcBuf[1]) >> 4;
            *(dst++) = tmp;

            tmp = sharpen::Base64DecodeMapping(srcBuf[1]) << 4;
            tmp |= sharpen::Base64DecodeMapping(srcBuf[2]) >> 2;
            *(dst++) = tmp;

            tmp = sharpen::Base64DecodeMapping(srcBuf[2]) << 6;
            tmp |= sharpen::Base64DecodeMapping(srcBuf[3]);
            *(dst++) = tmp;
            i = 0;
        }
    }
    return true;
}

std::uint32_t sharpen::BufferHash32(const char *data,std::size_t size) noexcept
{
    return sharpen::BufferHash32(data,data + size);
} 

std::uint64_t sharpen::BufferHash64(const char *data,std::size_t size) noexcept
{
    return sharpen::BufferHash64(data,data + size);
}

std::int32_t sharpen::BufferCompare(const char *leftBuf,std::size_t leftSize,const char *rightBuf,std::size_t rightSize) noexcept
{
    std::int32_t r{0};
    if(leftBuf && rightBuf)
    {
        r = std::memcmp(leftBuf,rightBuf,(std::min)(leftSize,rightSize));
    }
    if(r != 0)
    {
        return r > 0 ? 1:-1;
    }
    if(leftSize > rightSize)
    {
        return 1;
    }
    else if(rightSize > leftSize)
    {
        return -1;
    }
    return 0;
}