#pragma once
#ifndef _SHARPEN_IRAFTMAILBUILDER_HPP
#define _SHARPEN_IRAFTMAILBUILDER_HPP

#include "Mail.hpp"
#include "RaftHeartbeatRequest.hpp"
#include "RaftHeartbeatResponse.hpp"
#include "RaftPrevoteRequest.hpp"
#include "RaftPrevoteResponse.hpp"
#include "RaftSnapshotRequest.hpp"
#include "RaftSnapshotResponse.hpp"
#include "RaftVoteForRequest.hpp"
#include "RaftVoteForResponse.hpp"

namespace sharpen
{
    class IRaftMailBuilder
    {
    private:
        using Self = sharpen::IRaftMailBuilder;

    protected:
    public:
        IRaftMailBuilder() noexcept = default;

        IRaftMailBuilder(const Self &other) noexcept = default;

        IRaftMailBuilder(Self &&other) noexcept = default;

        Self &operator=(const Self &other) noexcept = default;

        Self &operator=(Self &&other) noexcept = default;

        virtual ~IRaftMailBuilder() noexcept = default;

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        virtual sharpen::Mail BuildVoteRequest(
            const sharpen::RaftVoteForRequest &request) const = 0;

        virtual sharpen::Mail BuildVoteResponse(
            const sharpen::RaftVoteForResponse &response) const = 0;

        virtual sharpen::Mail BuildHeartbeatRequest(
            const sharpen::RaftHeartbeatRequest &request) const = 0;

        virtual sharpen::Mail BuildHeartbeatResponse(
            const sharpen::RaftHeartbeatResponse &response) const = 0;

        virtual sharpen::Mail BuildPrevoteRequest(
            const sharpen::RaftPrevoteRequest &request) const = 0;

        virtual sharpen::Mail BuildPrevoteResponse(
            const sharpen::RaftPrevoteResponse &response) const = 0;

        virtual sharpen::Mail BuildSnapshotRequest(
            const sharpen::RaftSnapshotRequest &request) const = 0;

        virtual sharpen::Mail BuildSnapshotResponse(
            const sharpen::RaftSnapshotResponse &response) const = 0;
    };
}   // namespace sharpen

#endif