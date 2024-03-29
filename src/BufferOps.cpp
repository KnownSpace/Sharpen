#include <sharpen/BufferOps.hpp>

#include <sharpen/IntOps.hpp>
#include <cassert>
#include <utility>

unsigned char sharpen::Crc16TableHeight[256] = {
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

    0x00, 0xC1, 0x81, 0x40};

unsigned char sharpen::Crc16TableLow[256] = {

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

    0x41, 0x81, 0x80, 0x40};

std::uint32_t sharpen::Crc32Table[256] = {
    0x0,        0x77073096, 0xEE0E612C, 0x990951BA, 0x76DC419,  0x706AF48F, 0xE963A535,

    0x9E6495A3, 0xEDB8832,  0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x9B64C2B,

    0x7EB17CBD, 0xE7B82D07, 0x90BF1D91, 0x1DB71064, 0x6AB020F2, 0xF3B97148,

    0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7, 0x136C9856,

    0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63,

    0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1,

    0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6,

    0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59, 0x26D930AC,

    0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599,

    0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87,

    0x58684C11, 0xC1611DAB, 0xB6662D3D, 0x76DC4190, 0x1DB7106,  0x98D220BC,

    0xEFD5102A, 0x71B18589, 0x6B6B51F,  0x9FBFE4A5, 0xE8B8D433, 0x7807C9A2,

    0xF00F934,  0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x86D3D2D,  0x91646C97,

    0xE6635C01, 0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED,

    0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA,

    0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65, 0x4DB26158,

    0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D,

    0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73,

    0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9, 0x5005713C, 0x270241AA, 0xBE0B1010,

    0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F, 0x5EDEF90E,

    0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B,

    0xC0BA6CAD, 0xEDB88320, 0x9ABFB3B6, 0x3B6E20C,  0x74B1D29A, 0xEAD54739,

    0x9DD277AF, 0x4DB2615,  0x73DC1683, 0xE3630B12, 0x94643B84, 0xD6D6A3E,

    0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0xA00AE27,  0x7D079EB1, 0xF00F9344,

    0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671,

    0x6E6B06E7, 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F,

    0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4,

    0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B, 0xD80D2BDA,

    0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF,

    0x4669BE79, 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795,

    0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92,

    0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D, 0x9B64C2B0,

    0xEC63F226, 0x756AA39C, 0x26D930A,  0x9C0906A9, 0xEB0E363F, 0x72076785,

    0x5005713,  0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0xCB61B38,  0x92D28E9B,

    0xE5D5BE0D, 0x7CDCEFB7, 0xBDBDF21,  0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8,

    0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777, 0x88085AE6,

    0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3,

    0x166CCF45, 0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661,

    0xD06016F7, 0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC, 0x40DF0B66,

    0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9, 0xBDBDF21C,

    0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729,

    0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37,

    0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D};

std::uint16_t sharpen::Crc16(const char *data, std::size_t size) noexcept {
    sharpen::UintUnion<16> crc;
    crc.value_ = 0xFFFF;
    for (const std::uint8_t *begin = reinterpret_cast<const std::uint8_t *>(data),
                            *end = begin + size;
         begin != end;
         ++begin) {
        std::uint8_t index{static_cast<std::uint8_t>(crc.union_.height_ ^ *begin)};
        crc.union_.height_ = crc.union_.low_ ^ Crc16TableHeight[index];
        crc.union_.low_ = Crc16TableLow[index];
    }
    std::swap(crc.union_.low_, crc.union_.height_);
    return crc.value_;
}

std::uint32_t sharpen::Crc32(const char *data, std::size_t size) noexcept {
    std::uint32_t crc = 0xFFFFFFFF;
    for (const std::uint8_t *begin = reinterpret_cast<const std::uint8_t *>(data),
                            *end = begin + size;
         begin != end;
         ++begin) {
        crc = sharpen::Crc32Table[(crc ^ *begin) & 0xFF] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFF;
}

std::uint32_t sharpen::Adler32(const char *data, std::size_t size) noexcept {
    sharpen::UintUnion<32> adler32{};
    adler32.union_.low_ = 1;
    for (const char *end = data + size; data != end; ++data) {
        adler32.union_.low_ += *data;
        adler32.union_.low_ %= 65521;
        adler32.union_.height_ += adler32.union_.low_;
        adler32.union_.height_ %= 65521;
    }
    return adler32.value_;
}

std::size_t sharpen::ComputeBase64EncodeSize(std::size_t size) noexcept {
    return size / 3 * 4 + ((size % 3) ? 4 : 0);
}

std::size_t sharpen::ComputeBase64DecodeSize(std::size_t size) noexcept {
    assert(size % 4 == 0);
    return size / 4 * 3;
}

bool sharpen::Base64Encode(char *dst,
                           std::size_t dstSize,
                           const char *src,
                           std::size_t srcSize) noexcept {
    if (dstSize < sharpen::ComputeBase64EncodeSize(srcSize)) {
        return false;
    }
    if (!srcSize) {
        return true;
    }
    static const char *base64Hex =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    unsigned char srcBuf[3] = {};
    const char *srcEnd = src + srcSize;
    std::size_t i{0};
    while (src != srcEnd) {
        srcBuf[i++] = static_cast<unsigned char>(*(src++));
        if (i == 3) {
            // convert to base 64
            *(dst++) = base64Hex[srcBuf[0] >> 2];
            *(dst++) = base64Hex[(srcBuf[0] & 0x03) << 4 | srcBuf[1] >> 4];
            *(dst++) = base64Hex[(srcBuf[1] & 0x0F) << 2 | srcBuf[2] >> 6];
            *(dst++) = base64Hex[srcBuf[2] & 0x3F];
            i = 0;
            continue;
        }
    }
    if (i != 0) {
        for (std::size_t j = i; j != 3; ++j) {
            srcBuf[j] = 0;
        }
        *(dst++) = base64Hex[srcBuf[0] >> 2];
        *(dst++) = base64Hex[(srcBuf[0] & 0x03) << 4 | srcBuf[1] >> 4];
        *(dst++) = i == 2 ? base64Hex[(srcBuf[1] & 0x0F) << 2 | srcBuf[2] >> 6] : '=';
        *(dst++) = '=';
    }
    return true;
}

char sharpen::Base64DecodeMapping(unsigned char c) noexcept {
    if (c >= 'A' && c <= 'Z') {
        return c - 'A';
    } else if (c >= 'a' && c <= 'z') {
        return c - 'a' + 26;
    } else if (c == '+') {
        return 62;
    } else if (c == '=') {
        return 0;
    } else if (c >= '0' && c <= '9') {
        return c - '0' + 52;
    }
    return 63;
}

bool sharpen::Base64Decode(char *dst,
                           std::size_t dstSize,
                           const char *src,
                           std::size_t srcSize) noexcept {
    if (dstSize < sharpen::ComputeBase64DecodeSize(srcSize) || srcSize % 4) {
        return false;
    }
    if (!srcSize) {
        return true;
    }
    unsigned char srcBuf[4] = {};
    const char *srcEnd = src + srcSize;
    std::size_t i{0};
    while (src != srcEnd) {
        srcBuf[i++] = static_cast<unsigned char>(*(src++));
        if (i == 4) {
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

std::uint32_t sharpen::BufferHash32(const char *data, std::size_t size) noexcept {
    return sharpen::BufferHash32(data, data + size);
}

std::uint64_t sharpen::BufferHash64(const char *data, std::size_t size) noexcept {
    return sharpen::BufferHash64(data, data + size);
}

std::int32_t sharpen::BufferCompare(const char *leftBuf,
                                    std::size_t leftSize,
                                    const char *rightBuf,
                                    std::size_t rightSize) noexcept {
    std::int32_t r{0};
    if (leftBuf && rightBuf) {
        r = std::memcmp(leftBuf, rightBuf, (std::min)(leftSize, rightSize));
    }
    if (r != 0) {
        return r > 0 ? 1 : -1;
    }
    if (leftSize > rightSize) {
        return 1;
    } else if (rightSize > leftSize) {
        return -1;
    }
    return 0;
}