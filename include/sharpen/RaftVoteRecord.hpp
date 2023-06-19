#pragma once
#ifndef _SHARPEN_RAFTVOTERECORD_HPP
#define _SHARPEN_RAFTVOTERECORD_HPP

#include "ActorId.hpp"
#include "BinarySerializable.hpp"
#include "CorruptedDataError.hpp"   // IWYU pragma: keep
#include <cstddef>
#include <cstdint>
#include <utility>


namespace sharpen {
    class RaftVoteRecord : public sharpen::BinarySerializable<sharpen::RaftVoteRecord> {
    private:
        using Self = sharpen::RaftVoteRecord;

        std::uint64_t term_;
        sharpen::ActorId actorId_;

    public:
        explicit RaftVoteRecord() noexcept = default;

        RaftVoteRecord(std::uint64_t term, const sharpen::ActorId &actorId) noexcept;

        RaftVoteRecord(const Self &other) noexcept = default;

        RaftVoteRecord(Self &&other) noexcept;

        inline Self &operator=(const Self &other) noexcept {
            if (this != std::addressof(other)) {
                Self tmp{other};
                std::swap(tmp, *this);
            }
            return *this;
        }

        Self &operator=(Self &&other) noexcept;

        ~RaftVoteRecord() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        inline std::uint64_t GetTerm() const noexcept {
            return this->term_;
        }

        inline void SetTerm(std::uint64_t term) noexcept {
            this->term_ = term;
        }

        inline sharpen::ActorId &ActorId() noexcept {
            return this->actorId_;
        }

        inline const sharpen::ActorId &ActorId() const noexcept {
            return this->actorId_;
        }

        std::size_t ComputeSize() const noexcept;

        std::size_t LoadFrom(const char *data, std::size_t size);

        std::size_t UnsafeStoreTo(char *data) const noexcept;
    };
}   // namespace sharpen

#endif