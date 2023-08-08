#pragma once
#ifndef _SHARPEN_IASYNCRANDOMWRITABLE_HPP
#define _SHARPEN_IASYNCRANDOMWRITABLE_HPP

#include "ByteBuffer.hpp"
#include "Future.hpp"
#include <cstddef>
#include <cstdint>

namespace sharpen {

    class IAsyncRandomWritable {
    private:
        using Self = sharpen::IAsyncRandomWritable;

    public:
        IAsyncRandomWritable() = default;

        IAsyncRandomWritable(const Self &) = default;

        IAsyncRandomWritable(Self &&) noexcept = default;

        virtual ~IAsyncRandomWritable() noexcept = default;

        virtual void WriteAsync(const char *buf,
                                std::size_t bufSize,
                                std::uint64_t offset,
                                sharpen::Future<std::size_t> &future) = 0;

        virtual void WriteAsync(const sharpen::ByteBuffer &buf,
                                std::size_t bufferOffset,
                                std::uint64_t offset,
                                sharpen::Future<std::size_t> &future) = 0;

        std::size_t WriteAsync(const char *buf, std::size_t bufSize, std::uint64_t offset);

        std::size_t WriteAsync(const sharpen::ByteBuffer &buf,
                               std::size_t bufferOffset,
                               std::uint64_t offset);

        std::size_t WriteAsync(const sharpen::ByteBuffer &buf, std::uint64_t offset);

        inline std::size_t WriteFixedAsync(const char *buf, std::size_t bufSize,std::uint64_t offset) {
            std::size_t off{0};
            while (off != bufSize) {
                std::size_t sz{this->WriteAsync(buf + off,bufSize - off,offset + off)};
                if (!sz) {
                    break;
                }
                off += sz;
            }
            return off;
        }

        inline std::size_t WriteFixedAsync(const sharpen::ByteBuffer &buf, std::size_t bufferOffset,std::uint64_t offset) {
            return this->WriteFixedAsync(buf.Data() + bufferOffset,buf.GetSize(),offset);
        }

        inline std::size_t WriteFixedAsync(const sharpen::ByteBuffer &buf,std::uint64_t offset) {
            return this->WriteFixedAsync(buf,0,offset);
        }

        template<typename _T,
                 typename _Check = sharpen::EnableIf<std::is_standard_layout<_T>::value>>
        inline std::size_t WriteObjectAsync(const _T &obj, std::uint64_t offset) {
            return this->WriteAsync(reinterpret_cast<const char *>(&obj), sizeof(obj), offset);
        }
    };
}   // namespace sharpen

#endif