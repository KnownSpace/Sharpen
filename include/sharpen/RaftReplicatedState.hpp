#pragma once
#ifndef _SHARPEN_RAFTREPLICATEDSTATE_HPP
#define _SHARPEN_RAFTREPLICATEDSTATE_HPP

#include "Noncopyable.hpp"
#include "Optional.hpp"
#include "RaftSnapshot.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>

namespace sharpen
{
    class RaftReplicatedState : public sharpen::Noncopyable
    {
    private:
        using Self = RaftReplicatedState;

        std::uint64_t matchIndex_;
        std::uint64_t nextIndex_;
        std::unique_ptr<sharpen::IRaftSnapshotChunk> snapshot_;
        sharpen::RaftSnapshotMetadata snapshotMetadata_;

    public:
        RaftReplicatedState() noexcept;

        explicit RaftReplicatedState(std::uint64_t matchIndex) noexcept;

        RaftReplicatedState(Self &&other) noexcept;

        Self &operator=(Self &&other) noexcept;

        ~RaftReplicatedState() noexcept = default;

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline std::uint64_t GetMatchIndex() const noexcept
        {
            return this->matchIndex_;
        }

        inline std::uint64_t GetNextIndex() const noexcept
        {
            return this->nextIndex_;
        }

        void Forward() noexcept;

        void Forward(std::uint64_t step) noexcept;

        void ForwardMatchPoint(std::uint64_t index) noexcept;

        void BackwardMatchPoint(std::uint64_t index) noexcept;

        void SetSnapshot(sharpen::RaftSnapshot snapshot) noexcept;

        const sharpen::IRaftSnapshotChunk *LookupSnapshot() const noexcept;

        sharpen::IRaftSnapshotChunk *LookupSnapshot() noexcept;

        sharpen::Optional<sharpen::RaftSnapshotMetadata> LookupSnapshotMetadata() const noexcept;
    };
}   // namespace sharpen

#endif