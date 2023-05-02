#pragma once
#ifndef _SHARPEN_SIGNALBUFFER_HPP
#define _SHARPEN_SIGNALBUFFER_HPP

#include "ByteBuffer.hpp"

namespace sharpen {
    class SignalBuffer {
    private:
        using Self = sharpen::SignalBuffer;

        sharpen::ByteBuffer buf_;
        std::size_t offset_;

    public:
        explicit SignalBuffer(std::size_t size);

        SignalBuffer(const Self &other) = default;

        SignalBuffer(Self &&other) noexcept;

        inline Self &operator=(const Self &other) {
            if (this != std::addressof(other)) {
                Self tmp{other};
                std::swap(tmp, *this);
            }
            return *this;
        }

        Self &operator=(Self &&other) noexcept;

        ~SignalBuffer() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        std::int32_t PopSignal() noexcept;

        inline std::size_t GetSize() const noexcept {
            return this->buf_.GetSize();
        }

        inline std::size_t GetSignalCount() const noexcept {
            return this->buf_.GetSize() - this->offset_;
        }

        inline char *Data() noexcept {
            return this->buf_.Data();
        }

        inline const char *Data() const noexcept {
            return this->buf_.Data();
        }
    };
}   // namespace sharpen

#endif