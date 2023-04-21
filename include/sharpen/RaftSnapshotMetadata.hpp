#pragma once
#ifndef _SHARPEN_RAFTSNAPSHOTMETADATA_HPP
#define _SHARPEN_RAFTSNAPSHOTMETADATA_HPP

#include "BinarySerializable.hpp"
#include <cstddef>
#include <cstdint>
#include <utility>

namespace sharpen
{
    class RaftSnapshotMetadata : public sharpen::BinarySerializable<sharpen::RaftSnapshotMetadata>
    {
    private:
        using Self = sharpen::RaftSnapshotMetadata;

        std::uint64_t lastIndex_;
        std::uint64_t lastTerm_;

    public:
        RaftSnapshotMetadata() noexcept;

        RaftSnapshotMetadata(std::uint64_t lastIndex, std::uint64_t lastTerm) noexcept;

        RaftSnapshotMetadata(const Self &other) noexcept;

        RaftSnapshotMetadata(Self &&other) noexcept;

        Self &operator=(const Self &other) noexcept;

        Self &operator=(Self &&other) noexcept;

        ~RaftSnapshotMetadata() noexcept = default;

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline std::uint64_t GetLastIndex() const noexcept
        {
            return this->lastIndex_;
        }

        inline std::uint64_t GetLastTerm() const noexcept
        {
            return this->lastTerm_;
        }

        void SetLastIndex(std::uint64_t index) noexcept;

        inline void SetLastTerm(std::uint64_t term) noexcept
        {
            this->lastTerm_ = term;
        }

        std::size_t ComputeSize() const noexcept;

        std::size_t UnsafeStoreTo(char *data) const noexcept;

        std::size_t LoadFrom(const char *data, std::size_t size);
    };
}   // namespace sharpen

#endif