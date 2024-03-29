#pragma once
#ifndef _SHARPEN_RAFTVOTEFORRESPONSE_HPP
#define _SHARPEN_RAFTVOTEFORRESPONSE_HPP

#include "BinarySerializable.hpp"
#include "CorruptedDataError.hpp"   // IWYU pragma: keep
#include <cstddef>
#include <cstdint>
#include <utility>

namespace sharpen {
    class RaftVoteForResponse : public sharpen::BinarySerializable<sharpen::RaftVoteForResponse> {
    private:
        using Self = sharpen::RaftVoteForResponse;

        bool status_;
        std::uint64_t term_;

    public:
        RaftVoteForResponse() noexcept;

        RaftVoteForResponse(bool status, std::uint64_t term) noexcept;

        RaftVoteForResponse(const Self &other) noexcept = default;

        RaftVoteForResponse(Self &&other) noexcept;

        inline Self &operator=(const Self &other) noexcept {
            if (this != std::addressof(other)) {
                Self tmp{other};
                std::swap(tmp, *this);
            }
            return *this;
        }

        Self &operator=(Self &&other) noexcept;

        ~RaftVoteForResponse() noexcept = default;

        std::size_t ComputeSize() const noexcept;

        std::size_t LoadFrom(const char *data, std::size_t size);

        std::size_t UnsafeStoreTo(char *data) const noexcept;

        inline const Self &Const() const noexcept {
            return *this;
        }

        inline bool GetStatus() const noexcept {
            return this->status_;
        }

        inline void SetStatus(bool status) noexcept {
            this->status_ = status;
        }

        inline std::uint64_t GetTerm() const noexcept {
            return this->term_;
        }

        inline void SetTerm(std::uint64_t term) noexcept {
            this->term_ = term;
        }
    };
}   // namespace sharpen

#endif