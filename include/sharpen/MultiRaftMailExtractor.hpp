#pragma once
#ifndef _SHARPEN_MULTIRAFTMAILEXTRACTOR_HPP
#define _SHARPEN_MULTIRAFTMAILEXTRACTOR_HPP

#include "IRaftMailExtractor.hpp"
#include "RaftMailType.hpp"
#include <cstddef>
#include <cstdint>

namespace sharpen {
    class MultiRaftMailExtractor : public sharpen::IRaftMailExtractor {
    private:
        using Self = sharpen::MultiRaftMailExtractor;

        std::uint32_t magic_;
        std::uint32_t raftNumber_;

        bool CheckMail(sharpen::RaftMailType expect, const sharpen::Mail &mail) const noexcept;

        virtual bool NviIsRaftMail(const sharpen::Mail &mail) const noexcept override;

        virtual sharpen::RaftMailType NviGetMailType(
            const sharpen::Mail &mail) const noexcept override;

        virtual sharpen::Optional<sharpen::RaftVoteForRequest> NviExtractVoteRequest(
            const sharpen::Mail &mail) const noexcept override;

        virtual sharpen::Optional<sharpen::RaftVoteForResponse> NviExtractVoteResponse(
            const sharpen::Mail &mail) const noexcept override;

        virtual sharpen::Optional<sharpen::RaftHeartbeatRequest> NviExtractHeartbeatRequest(
            const sharpen::Mail &mail) const noexcept override;

        virtual sharpen::Optional<sharpen::RaftHeartbeatResponse> NviExtractHeartbeatResponse(
            const sharpen::Mail &mail) const noexcept override;

        virtual sharpen::Optional<sharpen::RaftPrevoteRequest> NviExtractPrevoteRequest(
            const sharpen::Mail &mail) const noexcept override;

        virtual sharpen::Optional<sharpen::RaftPrevoteResponse> NviExtractPrevoteResponse(
            const sharpen::Mail &mail) const noexcept override;

        virtual sharpen::Optional<sharpen::RaftSnapshotRequest> NviExtractSnapshotRequest(
            const sharpen::Mail &mail) const noexcept override;

        virtual sharpen::Optional<sharpen::RaftSnapshotResponse> NviExtractSnapshotResponse(
            const sharpen::Mail &mail) const noexcept override;

    public:
        MultiRaftMailExtractor(std::uint32_t magic, std::uint32_t raftNumber) noexcept;

        MultiRaftMailExtractor(const Self &other) noexcept = default;

        MultiRaftMailExtractor(Self &&other) noexcept;

        inline Self &operator=(const Self &other) noexcept {
            if (this != std::addressof(other)) {
                Self tmp{other};
                std::swap(tmp, *this);
            }
            return *this;
        }

        Self &operator=(Self &&other) noexcept;

        virtual ~MultiRaftMailExtractor() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }
    };
}   // namespace sharpen

#endif