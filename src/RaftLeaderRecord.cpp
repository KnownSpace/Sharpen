#include <sharpen/RaftLeaderRecord.hpp>

#include <mutex>
#include <sharpen/ConsensusWriter.hpp>

sharpen::RaftLeaderRecord::RaftLeaderRecord() noexcept
    : lock_()
    , term_(sharpen::ConsensusWriter::noneEpoch)
    , leaderId_() {
}

sharpen::RaftLeaderRecord::RaftLeaderRecord(std::uint64_t term,
                                            const sharpen::ActorId &leaderId) noexcept
    : lock_()
    , term_(term)
    , leaderId_(leaderId) {
}

sharpen::ConsensusWriter sharpen::RaftLeaderRecord::GetRecord() const noexcept {
    {
        std::unique_lock<sharpen::SpinLock> lock{this->lock_};
        sharpen::ConsensusWriter record;
        record.SetEpoch(this->term_);
        record.WriterId() = this->leaderId_;
        return record;
    }
}

void sharpen::RaftLeaderRecord::Flush(std::uint64_t term,
                                      const sharpen::ActorId &leaderId) noexcept {
    {
        std::unique_lock<sharpen::SpinLock> lock{this->lock_};
        this->term_ = term;
        this->leaderId_ = leaderId;
    }
}