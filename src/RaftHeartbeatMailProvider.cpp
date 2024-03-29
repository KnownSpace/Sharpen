#include <sharpen/RaftHeartbeatMailProvider.hpp>

#include <sharpen/ConsensusWriter.hpp>
#include <sharpen/IntOps.hpp>


sharpen::RaftHeartbeatMailProvider::RaftHeartbeatMailProvider(
    const sharpen::ActorId &id,
    const sharpen::IRaftMailBuilder &builder,
    const sharpen::ILogStorage &log,
    sharpen::IRaftLogAccesser &logAccesser,
    sharpen::IRaftSnapshotProvider *snapshotProvider)
    : Self{id, builder, log, logAccesser, snapshotProvider, defaultBatchSize_} {
}

sharpen::RaftHeartbeatMailProvider::RaftHeartbeatMailProvider(
    const sharpen::ActorId &id,
    const sharpen::IRaftMailBuilder &builder,
    const sharpen::ILogStorage &log,
    sharpen::IRaftLogAccesser &logAccesser,
    sharpen::IRaftSnapshotProvider *snapshotProvider,
    std::uint32_t batchSize)
    : Self{id, builder, log, logAccesser, snapshotProvider,batchSize,Self::defaultEntiresSize_} {
}

sharpen::RaftHeartbeatMailProvider::RaftHeartbeatMailProvider(
    const sharpen::ActorId &id,
    const sharpen::IRaftMailBuilder &builder,
    const sharpen::ILogStorage &log,
    sharpen::IRaftLogAccesser &logAccesser,
    sharpen::IRaftSnapshotProvider *snapshotProvider,
    std::uint32_t batchSize,
    std::uint32_t entiresSize)
    : id_(id)
    , builder_(&builder)
    , logs_(&log)
    , logAccesser_(&logAccesser)
    , snapshotProvider_(snapshotProvider)
    , batchSize_(batchSize)
    , entiresSize_(entiresSize)
    , states_()
    , term_(sharpen::ConsensusWriter::noneEpoch)
    , round_(0)
    , commitIndex_(0) {
    assert(this->batchSize_ >= 1);
    if (this->batchSize_ < 1) {
        this->batchSize_ = 1;
    }
    if (this->entiresSize_ < Self::minEntiresSize_) {
        this->entiresSize_ = Self::minEntiresSize_;
    }
    if (this->entiresSize_ > Self::maxEntiresSize_) {
        this->entiresSize_ = Self::maxEntiresSize_;
    }
}

sharpen::RaftHeartbeatMailProvider::RaftHeartbeatMailProvider(Self &&other) noexcept
    : id_(std::move(other.id_))
    , builder_(other.builder_)
    , logs_(other.logs_)
    , snapshotProvider_(other.snapshotProvider_)
    , batchSize_(other.batchSize_)
    , entiresSize_(other.entiresSize_)
    , states_(std::move(other.states_))
    , term_(other.term_)
    , round_(other.round_)
    , commitIndex_(other.commitIndex_.load()) {
    other.builder_ = nullptr;
    other.logs_ = nullptr;
    other.snapshotProvider_ = nullptr;
    other.batchSize_ = Self::defaultBatchSize_;
    other.entiresSize_ = Self::defaultEntiresSize_;
    other.term_ = sharpen::ConsensusWriter::noneEpoch;
    other.round_ = 0;
    other.commitIndex_ = 0;
}

sharpen::RaftHeartbeatMailProvider &sharpen::RaftHeartbeatMailProvider::operator=(
    Self &&other) noexcept {
    if (this != std::addressof(other)) {
        this->id_ = std::move(other.id_);
        this->builder_ = other.builder_;
        this->logs_ = other.logs_;
        this->snapshotProvider_ = other.snapshotProvider_;
        this->batchSize_ = other.batchSize_;
        this->entiresSize_ = other.entiresSize_;
        this->states_ = std::move(other.states_);
        this->term_ = other.term_;
        this->round_ = other.round_;
        this->commitIndex_ = other.commitIndex_.load();
        other.builder_ = nullptr;
        other.logs_ = nullptr;
        other.snapshotProvider_ = nullptr;
        other.batchSize_ = Self::defaultBatchSize_;
        other.entiresSize_ = Self::defaultEntiresSize_;
        other.term_ = sharpen::ConsensusWriter::noneEpoch;
        other.round_ = 0;
        other.commitIndex_ = 0;
    }
    return *this;
}

const sharpen::RaftReplicatedState *sharpen::RaftHeartbeatMailProvider::LookupState(
    const sharpen::ActorId &actorId) const noexcept {
    auto ite = this->states_.find(actorId);
    if (ite != this->states_.end()) {
        return &ite->second;
    }
    return nullptr;
}

sharpen::Optional<std::uint64_t> sharpen::RaftHeartbeatMailProvider::LookupMatchIndex(
    const sharpen::ActorId &actorId) const noexcept {
    const sharpen::RaftReplicatedState *state{this->LookupState(actorId)};
    if (state) {
        return state->GetMatchIndex();
    }
    return sharpen::EmptyOpt;
}

sharpen::RaftReplicatedState *sharpen::RaftHeartbeatMailProvider::LookupMutableState(
    const sharpen::ActorId &actorId) const noexcept {
    auto ite = this->states_.find(actorId);
    if (ite != this->states_.end()) {
        return &ite->second;
    }
    return nullptr;
}

void sharpen::RaftHeartbeatMailProvider::RemoveState(const sharpen::ActorId &actorId) noexcept {
    auto ite = this->states_.find(actorId);
    if (ite != this->states_.end()) {
        this->states_.erase(ite);
    }
}

std::size_t sharpen::RaftHeartbeatMailProvider::GetSize() const noexcept {
    return this->states_.size();
}

bool sharpen::RaftHeartbeatMailProvider::Empty() const noexcept {
    return this->states_.empty();
}

void sharpen::RaftHeartbeatMailProvider::Clear() noexcept {
    return this->states_.clear();
}

sharpen::Optional<std::uint64_t>
sharpen::RaftHeartbeatMailProvider::GetSynchronizedIndex() const noexcept {
    if (!this->states_.empty()) {
        auto begin = this->states_.begin();
        auto end = this->states_.end();
        std::uint64_t index{begin->second.GetNextIndex()};
        if (begin->second.LookupSnapshot()) {
            return sharpen::EmptyOpt;
        }
        ++begin;
        while (begin != end) {
            if (begin->second.GetNextIndex() != index || begin->second.LookupSnapshot()) {
                return sharpen::EmptyOpt;
            }
            ++begin;
        }
        return index;
    }
    return this->GetCommitIndex();
}

void sharpen::RaftHeartbeatMailProvider::ReComputeCommitIndex() noexcept {
    std::uint64_t index{this->commitIndex_};
    if (!this->states_.empty()) {
        std::size_t majority{this->states_.size() + 1};
        majority /= 2;
        // nested loop aggregation
        for (auto begin = this->states_.begin(), end = this->states_.end(); begin != end; ++begin) {
            std::uint64_t matchIndex{begin->second.GetMatchIndex()};
            if (matchIndex > index) {
                std::size_t count{1};
                for (auto ite = this->states_.begin(); ite != end; ++ite) {
                    if (ite != begin && ite->second.GetMatchIndex() >= matchIndex) {
                        count += 1;
                    }
                }
                if (count >= majority) {
                    index = matchIndex;
                }
            }
        }
        if (index > this->commitIndex_) {
            this->commitIndex_ = index;
        }
    }
}

std::uint64_t sharpen::RaftHeartbeatMailProvider::GetCommitIndex() const noexcept {
    return this->commitIndex_;
}

void sharpen::RaftHeartbeatMailProvider::SetCommitIndex(std::uint64_t index) noexcept {
    std::uint64_t commitIndex{this->GetCommitIndex()};
    if (commitIndex < index) {
        for (auto begin = this->states_.begin(), end = this->states_.end(); begin != end; ++begin) {
            if (begin->second.GetMatchIndex() < index) {
                begin->second.ForwardMatchPoint(index);
            }
        }
        this->ReComputeCommitIndex();
    }
}

sharpen::Mail sharpen::RaftHeartbeatMailProvider::ProvideSnapshotRequest(
    sharpen::RaftReplicatedState *state) const {
    assert(this->snapshotProvider_ != nullptr);
    if (!this->snapshotProvider_) {
        // snapshot already disable
        return sharpen::Mail{};
    }
    assert(state != nullptr);
    sharpen::IRaftSnapshotChunk *snapshot{state->LookupSnapshot()};
    sharpen::Optional<sharpen::RaftSnapshotMetadata> metadataOpt{state->LookupSnapshotMetadata()};
    assert(metadataOpt.Exist());
    assert(snapshot != nullptr);
    if (snapshot == nullptr || !metadataOpt.Exist()) {
        return sharpen::Mail{};
    }
    sharpen::RaftSnapshotMetadata metadata{metadataOpt.Get()};
    sharpen::RaftSnapshotRequest request;
    request.SetTerm(this->term_);
    request.LeaderActorId() = this->id_;
    request.SetOffset(snapshot->GetOffset());
    request.SetLast(!snapshot->Forwardable());
    request.Metadata() = metadata;
    request.Data() = snapshot->GenerateChunkData();
    request.SetLeaseRound(this->round_);
    return this->builder_->BuildSnapshotRequest(request);
}

void sharpen::RaftHeartbeatMailProvider::Register(const sharpen::ActorId &actorId) {
    auto ite = this->states_.find(actorId);
    if (ite == this->states_.end()) {
        sharpen::RaftReplicatedState state{this->GetCommitIndex()};
        this->states_.emplace(actorId, std::move(state));
    }
}

sharpen::Optional<std::uint64_t> sharpen::RaftHeartbeatMailProvider::LookupTerm(
    std::uint64_t index) const noexcept {
    assert(this->logAccesser_ != nullptr);
    assert(this->logs_ != nullptr);
    sharpen::Optional<sharpen::ByteBuffer> entry{this->logs_->Lookup(index)};
    if (entry.Exist()) {
        return this->logAccesser_->LookupTerm(entry.Get());
    }
    return sharpen::EmptyOpt;
}

sharpen::Mail sharpen::RaftHeartbeatMailProvider::Provide(const sharpen::ActorId &actorId) const {
    assert(this->builder_ != nullptr);
    assert(this->logs_ != nullptr);
    assert(this->batchSize_ >= 1);
    // lookup next index
    sharpen::RaftReplicatedState *state{this->LookupMutableState(actorId)};
    assert(state != nullptr);
    if (!state) {
        return sharpen::Mail{};
    }
    if (state->LookupSnapshot()) {
        return this->ProvideSnapshotRequest(state);
    }
    // get next index
    std::uint64_t nextIndex{state->GetNextIndex()};
    // get last index of current logs
    std::uint64_t lastIndex{this->logs_->GetLastIndex()};
    // compute pre index
    std::uint64_t preIndex{nextIndex};
    if (preIndex != sharpen::ILogStorage::noneIndex) {
        preIndex -= 1;
    }
    // compute logs size
    std::uint64_t size{lastIndex - preIndex};
    // limit logs <= batchSize
    size = (std::min)(static_cast<std::uint64_t>(this->batchSize_), size);
    lastIndex = preIndex + size;
    sharpen::RaftHeartbeatRequest request;
    // get commit index
    std::uint64_t commitIndex{(std::min)(this->GetCommitIndex(), lastIndex)};
    request.SetCommitIndex(commitIndex);
    request.LeaderActorId() = this->id_;
    request.SetTerm(this->term_);
    request.SetLeaseRound(this->round_);
    request.SetPreLogIndex(preIndex);
    request.SetPreLogTerm(sharpen::ConsensusWriter::noneEpoch);
    sharpen::Optional<std::uint64_t> term{this->LookupTerm(preIndex)};
    if (!term.Exist() && preIndex != sharpen::ILogStorage::noneIndex) {
        assert(this->snapshotProvider_ != nullptr);
        if (!this->snapshotProvider_) {
            return sharpen::Mail{};
        }
        sharpen::RaftSnapshot snapshot{this->snapshotProvider_->GetSnapshot()};
        state->SetSnapshot(std::move(snapshot));
        return this->ProvideSnapshotRequest(state);
    }
    if (term.Exist()) {
        request.SetPreLogTerm(term.Get());
    }
    if (size) {
        // append log entires to request
        request.Entries().Reserve(sharpen::IntCast<std::size_t>(size));
        if (nextIndex == sharpen::ILogStorage::noneIndex) {
            nextIndex += 1;
        }
        std::size_t entiresSize{0};
        size = 0;
        while (nextIndex <= lastIndex) {
            sharpen::Optional<sharpen::ByteBuffer> log{this->logs_->Lookup(nextIndex)};
            if (!log.Exist()) {
                assert(this->snapshotProvider_ != nullptr);
                if (!this->snapshotProvider_) {
                    return sharpen::Mail{};
                }
                sharpen::RaftSnapshot snapshot{this->snapshotProvider_->GetSnapshot()};
                state->SetSnapshot(std::move(snapshot));
                return this->ProvideSnapshotRequest(state);
            }
            if (log.Get().GetSize() + entiresSize > this->entiresSize_) {
                break;
            }
            request.Entries().Push(std::move(log.Get()));
            nextIndex += 1;
            size += 1;
        }
        // forward state
        state->Forward(size);
    }
    return this->builder_->BuildHeartbeatRequest(request);
}

void sharpen::RaftHeartbeatMailProvider::PrepareTerm(std::uint64_t term) noexcept {
    this->term_ = term;
}

void sharpen::RaftHeartbeatMailProvider::PrepareRound(std::uint64_t round) noexcept {
    this->round_ = round;
}

sharpen::Mail sharpen::RaftHeartbeatMailProvider::ProvideSynchronizedMail() const {
    assert(!this->states_.empty());
    sharpen::ActorId actorId{this->states_.begin()->first};
    return this->Provide(actorId);
}

void sharpen::RaftHeartbeatMailProvider::ForwardState(const sharpen::ActorId &actorId,
                                                      std::uint64_t index) noexcept {
    sharpen::RaftReplicatedState *state{this->LookupMutableState(actorId)};
    assert(state != nullptr);
    if (state) {
        state->ForwardMatchPoint(index);
        this->ReComputeCommitIndex();
    }
}

void sharpen::RaftHeartbeatMailProvider::BackwardState(const sharpen::ActorId &actorId,
                                                       std::uint64_t index) noexcept {
    sharpen::RaftReplicatedState *state{this->LookupMutableState(actorId)};
    assert(state != nullptr);
    if (state) {
        state->BackwardMatchPoint(index);
    }
}