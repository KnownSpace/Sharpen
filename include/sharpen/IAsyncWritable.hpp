#pragma once
#ifndef _SHARPEN_IASYNCWRITABLE_HPP
#define _SHARPEN_IASYNCWRITABLE_HPP

#include "ByteBuffer.hpp"
#include "Future.hpp"
#include <cstddef>
#include <cstdint>

namespace sharpen {

    class IAsyncWritable {
    private:
        using Self = sharpen::IAsyncWritable;

    public:
        IAsyncWritable() = default;

        IAsyncWritable(const Self &) = default;

        IAsyncWritable(Self &&) noexcept = default;

        virtual ~IAsyncWritable() = default;

        virtual void WriteAsync(const char *buf,
                                std::size_t bufSize,
                                sharpen::Future<std::size_t> &future) = 0;

        virtual void WriteAsync(const sharpen::ByteBuffer &buf,
                                std::size_t bufferOffset,
                                sharpen::Future<std::size_t> &future) = 0;

        std::size_t WriteAsync(const char *buf, std::size_t bufSize);

        std::size_t WriteAsync(const sharpen::ByteBuffer &buf, std::size_t bufferOffset);

        std::size_t WriteAsync(const sharpen::ByteBuffer &buf);

        inline std::size_t WriteFixedAsync(const char *buf, std::size_t bufSize) {
            std::size_t off{0};
            while (off != bufSize) {
                std::size_t sz{this->WriteAsync(buf + off,bufSize - off)};
                if (!sz) {
                    break;
                }
                off += sz;
            }
            return off;
        }

        inline std::size_t WriteFixedAsync(const sharpen::ByteBuffer &buf, std::size_t bufferOffset) {
            return this->WriteFixedAsync(buf.Data() + bufferOffset,buf.GetSize());
        }

        inline std::size_t WriteFixedAsync(const sharpen::ByteBuffer &buf) {
            return this->WriteFixedAsync(buf,0);
        }

        template<typename _T,
                 typename _Check = sharpen::EnableIf<std::is_standard_layout<_T>::value>>
        inline std::size_t WriteObjectAsync(const _T &obj) {
            return this->WriteAsync(reinterpret_cast<const char *>(&obj), sizeof(obj));
        }
    };
}   // namespace sharpen

#endif
