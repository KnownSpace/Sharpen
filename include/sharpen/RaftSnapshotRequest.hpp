#pragma once
#ifndef _SHARPEN_RAFTSNAPSHOTREQUEST_HPP
#define _SHARPEN_RAFTSNAPSHOTREQUEST_HPP

#include "ActorId.hpp"
#include "BinarySerializable.hpp"
#include "ByteBuffer.hpp"
#include "RaftSnapshotMetadata.hpp"
#include <cstddef>
#include <cstdint>
#include <utility>


namespace sharpen {
    class RaftSnapshotRequest : public sharpen::BinarySerializable<sharpen::RaftSnapshotRequest> {
    private:
        using Self = sharpen::RaftSnapshotRequest;

        std::uint64_t term_;
        sharpen::ActorId leaderActorId_;
        std::uint64_t offset_;
        bool last_;
        sharpen::RaftSnapshotMetadata metadata_;
        sharpen::ByteBuffer data_;

    public:
        RaftSnapshotRequest() noexcept;

        RaftSnapshotRequest(const Self &other) = default;

        RaftSnapshotRequest(Self &&other) noexcept;

        inline Self &operator=(const Self &other) {
            if (this != std::addressof(other)) {
                Self tmp{other};
                std::swap(tmp, *this);
            }
            return *this;
        }

        Self &operator=(Self &&other) noexcept;

        ~RaftSnapshotRequest() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        inline sharpen::RaftSnapshotMetadata &Metadata() noexcept {
            return this->metadata_;
        }

        inline const sharpen::RaftSnapshotMetadata &Metadata() const noexcept {
            return this->metadata_;
        }

        inline sharpen::ByteBuffer &Data() noexcept {
            return this->data_;
        }

        inline const sharpen::ByteBuffer &Data() const noexcept {
            return this->data_;
        }

        inline bool IsLast() const noexcept {
            return this->last_;
        }

        inline void SetLast(bool last) noexcept {
            this->last_ = last;
        }

        inline std::uint64_t GetTerm() const noexcept {
            return this->term_;
        }

        inline void SetTerm(std::uint64_t term) noexcept {
            this->term_ = term;
        }

        inline sharpen::ActorId &LeaderActorId() noexcept {
            return this->leaderActorId_;
        }

        inline const sharpen::ActorId &LeaderActorId() const noexcept {
            return this->leaderActorId_;
        }

        inline std::uint64_t GetOffset() const noexcept {
            return this->offset_;
        }

        inline void SetOffset(std::uint64_t offset) noexcept {
            this->offset_ = offset;
        }

        std::size_t ComputeSize() const noexcept;

        std::size_t UnsafeStoreTo(char *data) const noexcept;

        std::size_t LoadFrom(const char *data, std::size_t size);
    };
}   // namespace sharpen

#endif