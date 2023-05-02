#pragma once
#ifndef _SHARPEN_RAFTPREVOTERESPONSE_HPP
#define _SHARPEN_RAFTPREVOTERESPONSE_HPP

#include "BinarySerializable.hpp"
#include <cstddef>
#include <cstdint>
#include <utility>

namespace sharpen {
    class RaftPrevoteResponse : public sharpen::BinarySerializable<sharpen::RaftPrevoteResponse> {
    private:
        using Self = sharpen::RaftPrevoteResponse;

        bool status_;
        std::uint64_t term_;

    public:
        RaftPrevoteResponse() noexcept;

        RaftPrevoteResponse(const Self &other) noexcept = default;

        RaftPrevoteResponse(Self &&other) noexcept;

        inline Self &operator=(const Self &other) noexcept {
            if (this != std::addressof(other)) {
                Self tmp{other};
                std::swap(tmp, *this);
            }
            return *this;
        }

        Self &operator=(Self &&other) noexcept;

        ~RaftPrevoteResponse() noexcept = default;

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

        std::size_t ComputeSize() const noexcept;

        std::size_t LoadFrom(const char *data, std::size_t size);

        std::size_t UnsafeStoreTo(char *data) const noexcept;
    };
}   // namespace sharpen

#endif