#pragma once
#ifndef _SHARPEN_CONSENSUSRESULT_HPP
#define _SHARPEN_CONSENSUSRESULT_HPP

#include "AtomicBitsetEnum.hpp"


namespace sharpen {
    enum class ConsensusResultEnum : std::uint32_t
    {
        None = 0,
        // log may commited, please check your log storage
        LogsCommit = 1,
        // a new sanpshot was received
        SnapshotReceived = 2,
        // consensus status changed, the writable actor has changed
        StatusChanged = 2 << 1,
        // lease confirmed by majority
        LeaseConfirmed = 2 << 2
    };

    class ConsensusResult
        : private sharpen::AtomicBitsetEnum<sharpen::ConsensusResultEnum,
                                            sharpen::ConsensusResultEnum::None> {
    private:
        using Self = sharpen::ConsensusResult;
        using Base = sharpen::AtomicBitsetEnum<sharpen::ConsensusResultEnum,
                                               sharpen::ConsensusResultEnum::None>;

        explicit ConsensusResult(Base impl) noexcept;
    public:
        ConsensusResult() noexcept = default;

        ConsensusResult(const Self &other) noexcept = default;

        ConsensusResult(Self &&other) noexcept = default;

        Self &operator=(const Self &other) noexcept = default;

        Self &operator=(Self &&other) noexcept = default;

        ~ConsensusResult() noexcept = default;

        inline const Self &Const() const noexcept {
            return *this;
        }

        void Set(sharpen::ConsensusResultEnum bit) noexcept;

        inline bool IsLogsCommit() const noexcept {
            return this->IsSet(sharpen::ConsensusResultEnum::LogsCommit);
        }

        inline bool IsSnapshotReceive() const noexcept {
            return this->IsSet(sharpen::ConsensusResultEnum::SnapshotReceived);
        }

        inline bool IsStatusChanged() const noexcept {
            return this->IsSet(sharpen::ConsensusResultEnum::StatusChanged);
        }

        inline bool IsLeaseConfirmed() const noexcept {
            return this->IsSet(sharpen::ConsensusResultEnum::LeaseConfirmed);
        }

        bool IsNone() const noexcept;

        Self Take() noexcept;
    };
}   // namespace sharpen

#endif