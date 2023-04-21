#pragma once
#ifndef _SHARPEN_RAFTSNAPSHOT_HPP
#define _SHARPEN_RAFTSNAPSHOT_HPP

#include "IRaftSnapshotChunk.hpp"
#include "Noncopyable.hpp"
#include "RaftSnapshotMetadata.hpp"
#include <memory>

namespace sharpen
{
    class RaftSnapshot : public sharpen::Noncopyable
    {
    private:
        using Self = sharpen::RaftSnapshot;

        std::unique_ptr<sharpen::IRaftSnapshotChunk> chunk_;
        sharpen::RaftSnapshotMetadata metadata_;

    public:
        explicit RaftSnapshot(std::unique_ptr<sharpen::IRaftSnapshotChunk> chunk) noexcept;

        RaftSnapshot(std::unique_ptr<sharpen::IRaftSnapshotChunk> chunk,
                     sharpen::RaftSnapshotMetadata metatdata) noexcept;

        RaftSnapshot(Self &&other) noexcept;

        Self &operator=(Self &&other) noexcept;

        ~RaftSnapshot() noexcept = default;

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline sharpen::RaftSnapshotMetadata &Metadata() noexcept
        {
            return this->metadata_;
        }

        inline const sharpen::RaftSnapshotMetadata &Metadata() const noexcept
        {
            return this->metadata_;
        }

        std::unique_ptr<sharpen::IRaftSnapshotChunk> ReleaseChainedChunks() noexcept;
    };
}   // namespace sharpen

#endif