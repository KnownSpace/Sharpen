#pragma once
#ifndef _SHARPEN_RAFTHEARTBEATREQUEST_HPP
#define _SHARPEN_RAFTHEARTBEATREQUEST_HPP

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include "BinarySerializable.hpp"
#include "ByteBuffer.hpp"
#include "LogEntries.hpp"

namespace sharpen
{
    class RaftHeartbeatRequest : public sharpen::BinarySerializable<sharpen::RaftHeartbeatRequest>
    {
    private:
        using Self = sharpen::RaftHeartbeatRequest;

        std::uint64_t term_;
        std::uint64_t leaderId_;
        std::uint64_t preLogIndex_;
        std::uint64_t preLogTerm_;
        sharpen::LogEntries entries_;
        std::uint64_t leaderCommitIndex_;

    public:
        RaftHeartbeatRequest() noexcept;

        RaftHeartbeatRequest(const Self &other) = default;

        RaftHeartbeatRequest(Self &&other) noexcept;

        inline Self &operator=(const Self &other)
        {
            if (this != std::addressof(other))
            {
                Self tmp{other};
                std::swap(tmp, *this);
            }
            return *this;
        }

        Self &operator=(Self &&other) noexcept;

        ~RaftHeartbeatRequest() noexcept = default;

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline std::uint64_t GetTerm() const noexcept
        {
            return this->term_;
        }

        inline void SetTerm(std::uint64_t term) noexcept
        {
            this->term_ = term;
        }

        inline std::uint64_t GetLeaderId() const noexcept
        {
            return this->leaderId_;
        }

        inline void SetLeaderId(std::uint64_t leaderId) noexcept
        {
            this->leaderId_ = leaderId;
        }

        inline std::uint64_t GetPreLogIndex() const noexcept
        {
            return this->preLogIndex_;
        }

        inline void SetPreLogIndex(std::uint64_t logIndex) noexcept
        {
            this->preLogIndex_ = logIndex;
        }

        inline std::uint64_t GetPreLogTerm() const noexcept
        {
            return this->preLogTerm_;
        }

        inline void SetPreLogTerm(std::uint64_t term) noexcept
        {
            this->preLogTerm_ = term;
        }

        inline sharpen::LogEntries &Entries() noexcept
        {
            return this->entries_;
        }

        inline const sharpen::LogEntries &Entries() const noexcept
        {
            return this->entries_;
        }

        inline std::uint64_t GetCommitIndex() const noexcept
        {
            return this->leaderCommitIndex_;
        }

        inline void SetCommitIndex(std::uint64_t commitIndex) noexcept
        {
            this->leaderCommitIndex_ = commitIndex;
        }

        std::size_t ComputeSize() const noexcept;

        std::size_t LoadFrom(const char *data, std::size_t size);

        std::size_t UnsafeStoreTo(char *data) const noexcept;
    };
}   // namespace sharpen

#endif