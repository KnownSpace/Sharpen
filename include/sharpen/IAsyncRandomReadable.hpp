#pragma once
#ifndef _SHARPEN_IASYNCRANDOMREADABLE_HPP
#define _SHARPEN_IASYNCRANDOMREADABLE_HPP

#include "ByteBuffer.hpp"
#include "Future.hpp"
#include <cstddef>
#include <cstdint>

namespace sharpen {

    class IAsyncRandomReadable {
    private:
        using Self = sharpen::IAsyncRandomReadable;

    public:
        IAsyncRandomReadable() = default;

        IAsyncRandomReadable(const Self &) = default;

        IAsyncRandomReadable(Self &&) noexcept = default;

        virtual ~IAsyncRandomReadable() noexcept = default;

        virtual void ReadAsync(char *buf,
                               std::size_t bufSize,
                               std::uint64_t offset,
                               sharpen::Future<std::size_t> &future) = 0;

        virtual void ReadAsync(sharpen::ByteBuffer &buf,
                               std::size_t bufferOffset,
                               std::uint64_t offset,
                               sharpen::Future<std::size_t> &future) = 0;

        std::size_t ReadAsync(char *buf, std::size_t bufSize, std::uint64_t offset);

        std::size_t ReadAsync(sharpen::ByteBuffer &buf,
                              std::size_t bufferOffset,
                              std::uint64_t offset);

        std::size_t ReadAsync(sharpen::ByteBuffer &buf, std::uint64_t offset);

        inline std::size_t ReadFixedAsync(char *buf, std::size_t size, std::uint64_t offset) {
            std::size_t off{0};
            while (off != size) {
                std::size_t sz{this->ReadAsync(buf + off, size - off,offset + off)};
                if (!sz) {
                    break;
                }
                off += sz;
            }
            return off;
        }

        inline std::size_t ReadFixedAsync(sharpen::ByteBuffer &buf, std::size_t bufOffset, std::uint64_t offset) {
            assert(buf.GetSize() >= bufOffset);
            return this->ReadFixedAsync(buf.Data() + bufOffset, buf.GetSize() - bufOffset,offset);
        }

        inline std::size_t ReadFixedAsync(sharpen::ByteBuffer &buf, std::uint64_t offset) {
            return this->ReadFixedAsync(buf, 0,offset);
        }

        template<typename _T,
                 typename _Check = sharpen::EnableIf<std::is_standard_layout<_T>::value>>
        inline std::size_t ReadObjectAsync(_T &obj, std::uint64_t offset) {
            return this->ReadAsync(reinterpret_cast<char *>(&obj), sizeof(obj), offset);
        }
    };
}   // namespace sharpen

#endif
