#pragma once
#ifndef _SHARPEN_RAFTMAILTYPE_HPP
#define _SHARPEN_RAFTMAILTYPE_HPP

#include <cstdint>
#include <cstddef>

namespace sharpen
{
    enum class RaftMailType:std::uint32_t
    {
        Unknown = 0,
        HeartbeatRequest = 1,
        HeartbeatResponse = 2,
        VoteRequest = 3,
        VoteResponse = 4,
        InstallSnapshotRequest = 5,
        InstallSnapshotResponse = 6,
        PreVoteRequest = 7,
        PreVoteResponse = 8,
        MaxValue = 9
    };

    constexpr inline static bool IsValiedRaftMailType(std::uint32_t type) noexcept
    {
        constexpr std::uint32_t minVal{static_cast<std::uint32_t>(RaftMailType::Unknown)};
        constexpr std::uint32_t MaxVal{static_cast<std::uint32_t>(RaftMailType::MaxValue)};
        return type > minVal && type < MaxVal;
    }
}

#endif