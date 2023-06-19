#pragma once
#ifndef _SHARPEN_RAFTLEADERRECORD_HPP
#define _SHARPEN_RAFTLEADERRECORD_HPP

#include "ActorId.hpp"
#include "ConsensusWriter.hpp"
#include "Noncopyable.hpp"
#include "Nonmovable.hpp"
#include "SpinLock.hpp"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <utility>


namespace sharpen {
    class RaftLeaderRecord
        : public sharpen::Noncopyable
        , public sharpen::Nonmovable {
    private:
        using Self = sharpen::RaftLeaderRecord;

        mutable sharpen::SpinLock lock_;
        std::atomic_uint64_t term_;
        sharpen::ActorId leaderId_;

    public:
        RaftLeaderRecord() noexcept;

        RaftLeaderRecord(std::uint64_t term, const sharpen::ActorId &leaderId) noexcept;

        ~RaftLeaderRecord() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        sharpen::ConsensusWriter GetRecord() const noexcept;

        void Flush(std::uint64_t term, const sharpen::ActorId &leaderId) noexcept;
    };
}   // namespace sharpen

#endif