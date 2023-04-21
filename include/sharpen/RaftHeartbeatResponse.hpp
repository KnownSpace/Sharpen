#pragma once
#ifndef _SHARPEN_RAFTHEARTBEATRESPONSE_HPP
#define _SHARPEN_RAFTHEARTBEATRESPONSE_HPP

#include "BinarySerializable.hpp"
#include <cstddef>
#include <cstdint>

namespace sharpen
{
    class RaftHeartbeatResponse : public sharpen::BinarySerializable<sharpen::RaftHeartbeatResponse>
    {
    private:
        using Self = sharpen::RaftHeartbeatResponse;

        std::uint8_t status_;
        std::uint64_t term_;
        std::uint64_t matchIndex_;

    public:
        RaftHeartbeatResponse() noexcept = default;

        explicit RaftHeartbeatResponse(bool status) noexcept;

        RaftHeartbeatResponse(const Self &other) noexcept = default;

        RaftHeartbeatResponse(Self &&other) noexcept;

        inline Self &operator=(const Self &other) noexcept
        {
            if (this != std::addressof(other))
            {
                Self tmp{other};
                std::swap(tmp, *this);
            }
            return *this;
        }

        Self &operator=(Self &&other) noexcept;

        ~RaftHeartbeatResponse() noexcept = default;

        inline const Self &Const() const noexcept
        {
            return *this;
        }

        inline bool GetStatus() const noexcept
        {
            return this->status_;
        }

        inline void SetStatus(bool status) noexcept
        {
            this->status_ = static_cast<std::uint8_t>(status);
        }

        inline std::uint64_t GetTerm() const noexcept
        {
            return this->term_;
        }

        inline void SetTerm(std::uint64_t term) noexcept
        {
            this->term_ = term;
        }

        inline std::uint64_t GetMatchIndex() const noexcept
        {
            return this->matchIndex_;
        }

        inline void SetMatchIndex(std::uint64_t matchIndex) noexcept
        {
            this->matchIndex_ = matchIndex;
        }

        std::size_t ComputeSize() const noexcept;

        std::size_t LoadFrom(const char *data, std::size_t size);

        std::size_t UnsafeStoreTo(char *data) const noexcept;
    };
}   // namespace sharpen

#endif