#include <sharpen/RaftLeaderRecord.hpp>

#include <mutex>

sharpen::RaftLeaderRecord::RaftLeaderRecord() noexcept
    : lock_()
    , term_(0)
    , leaderId_() {
}

sharpen::RaftLeaderRecord::RaftLeaderRecord(std::uint64_t term,
                                            const sharpen::ActorId &leaderId) noexcept
    : lock_()
    , term_(term)
    , leaderId_(leaderId) {
}

std::pair<std::uint64_t, sharpen::ActorId> sharpen::RaftLeaderRecord::GetRecord() const noexcept {
    {
        std::unique_lock<sharpen::SpinLock> lock{this->lock_};
        return {this->term_, this->leaderId_};
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