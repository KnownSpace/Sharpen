#pragma once
#ifndef _SHARPEN_RAFTHEARTBEATREQUEST_HPP
#define _SHARPEN_RAFTHEARTBEATREQUEST_HPP

#include "ActorId.hpp"
#include "BinarySerializable.hpp"
#include "LogEntries.hpp"
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>


namespace sharpen {
    class RaftHeartbeatRequest : public sharpen::BinarySerializable<sharpen::RaftHeartbeatRequest> {
    private:
        using Self = sharpen::RaftHeartbeatRequest;

        std::uint64_t term_;
        sharpen::ActorId leaderActorId_;
        std::uint64_t preLogIndex_;
        std::uint64_t preLogTerm_;
        sharpen::LogEntries entries_;
        std::uint64_t leaderCommitIndex_;
        std::uint64_t leaseRound_;

    public:
        RaftHeartbeatRequest() noexcept;

        RaftHeartbeatRequest(const Self &other) = default;

        RaftHeartbeatRequest(Self &&other) noexcept;

        inline Self &operator=(const Self &other) {
            if (this != std::addressof(other)) {
                Self tmp{other};
                std::swap(tmp, *this);
            }
            return *this;
        }

        Self &operator=(Self &&other) noexcept;

        ~RaftHeartbeatRequest() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
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

        inline std::uint64_t GetPreLogIndex() const noexcept {
            return this->preLogIndex_;
        }

        inline void SetPreLogIndex(std::uint64_t logIndex) noexcept {
            this->preLogIndex_ = logIndex;
        }

        inline std::uint64_t GetPreLogTerm() const noexcept {
            return this->preLogTerm_;
        }

        inline void SetPreLogTerm(std::uint64_t term) noexcept {
            this->preLogTerm_ = term;
        }

        inline sharpen::LogEntries &Entries() noexcept {
            return this->entries_;
        }

        inline const sharpen::LogEntries &Entries() const noexcept {
            return this->entries_;
        }

        inline std::uint64_t GetCommitIndex() const noexcept {
            return this->leaderCommitIndex_;
        }

        inline void SetCommitIndex(std::uint64_t commitIndex) noexcept {
            this->leaderCommitIndex_ = commitIndex;
        }

        inline std::uint64_t GetLeaseRound() const noexcept {
            return this->leaseRound_;
        }

        inline void SetLeaseRound(std::uint64_t leaseRound) noexcept {
            this->leaseRound_ = leaseRound;
        }

        std::size_t ComputeSize() const noexcept;

        std::size_t LoadFrom(const char *data, std::size_t size);

        std::size_t UnsafeStoreTo(char *data) const noexcept;
    };
}   // namespace sharpen

#endif