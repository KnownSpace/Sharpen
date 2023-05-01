#pragma once
#ifndef _SHARPEN_RAFTPREVOTEREQUEST_HPP
#define _SHARPEN_RAFTPREVOTEREQUEST_HPP

#include "BinarySerializable.hpp"
#include <cstddef>
#include <cstdint>
#include <utility>

namespace sharpen {
    class RaftPrevoteRequest : public sharpen::BinarySerializable<sharpen::RaftPrevoteRequest> {
    private:
        using Self = sharpen::RaftPrevoteRequest;

        std::uint64_t lastIndex_;
        std::uint64_t lastTerm_;

    public:
        RaftPrevoteRequest() noexcept;

        RaftPrevoteRequest(const Self &other) noexcept = default;

        RaftPrevoteRequest(Self &&other) noexcept;

        inline Self &operator=(const Self &other) noexcept {
            if (this != std::addressof(other)) {
                Self tmp{other};
                std::swap(tmp, *this);
            }
            return *this;
        }

        Self &operator=(Self &&other) noexcept;

        ~RaftPrevoteRequest() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        inline std::uint64_t GetLastIndex() const noexcept {
            return this->lastIndex_;
        }

        inline void SetLastIndex(std::uint64_t index) noexcept {
            this->lastIndex_ = index;
        }

        inline std::uint64_t GetLastTerm() const noexcept {
            return this->lastTerm_;
        }

        inline void SetLastTerm(std::uint64_t term) noexcept {
            this->lastTerm_ = term;
        }

        std::size_t ComputeSize() const noexcept;

        std::size_t LoadFrom(const char *data, std::size_t size);

        std::size_t UnsafeStoreTo(char *data) const noexcept;
    };
}   // namespace sharpen

#endif