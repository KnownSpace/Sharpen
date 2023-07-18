#include <sharpen/RaftConsensus.hpp>

#include <sharpen/BufferReader.hpp>
#include <sharpen/BufferWriter.hpp>
#include <sharpen/LogEntries.hpp>
#include <sharpen/SingleWorkerGroup.hpp>
#include <sharpen/SystemError.hpp>
#include <cassert>
#include <utility>

#ifndef _NDEBUG
#include <sharpen/DebugTools.hpp>
#endif

sharpen::RaftConsensus::RaftConsensus(
    const sharpen::ActorId &id,
    std::unique_ptr<sharpen::IStatusMap> statusMap,
    std::unique_ptr<sharpen::ILogStorage> logs,
    std::unique_ptr<sharpen::IRaftLogAccesser> logAccesser,
    std::unique_ptr<sharpen::IRaftSnapshotController> snapshotController,
    std::shared_ptr<sharpen::RaftLeaderCounter> leaderCount,
    const sharpen::RaftOption &option,
    sharpen::IFiberScheduler &scheduler)
    : scheduler_(&scheduler)
    , id_(id)
    , statusMap_(std::move(statusMap))
    , logs_(std::move(logs))
    , logAccesser_(std::move(logAccesser))
    , snapshotController_(std::move(snapshotController))
    , option_(option)
    , term_(sharpen::ConsensusWriter::noneEpoch)
    , vote_(sharpen::ConsensusWriter::noneEpoch, sharpen::ActorId{})
    , role_(sharpen::RaftRole::Follower)
    , electionRecord_(sharpen::ConsensusWriter::noneEpoch, 0)
    , prevoteRecord_()
    , leaseStatus_()
    , prevLeaderCount_(0)
    , leaderCount_(std::move(leaderCount))
    , leaderRecord_(sharpen::ConsensusWriter::noneEpoch, sharpen::ActorId{})
    , lastResult_()
    , waiter_(nullptr)
    , advancedCount_(0)
    , reachAdvancedCount_(0)
    , mailBuilder_(nullptr)
    , mailExtractor_(nullptr)
    , peers_(nullptr)
    , peersBroadcaster_(nullptr)
    , heartbeatProvider_(nullptr)
    , worker_(nullptr) {
    // check pointers
    assert(this->statusMap_ != nullptr);
    assert(this->logs_ != nullptr);
    assert(this->logAccesser_ != nullptr);
    // set worker
    sharpen::IWorkerGroup *worker{new (std::nothrow) sharpen::SingleWorkerGroup{*this->scheduler_}};
    if (!worker) {
        throw std::bad_alloc{};
    }
    this->worker_.reset(worker);
    // load status cache
    this->LoadTerm();
    this->LoadVoteFor();
    // set learner if need
    if (this->option_.IsLearner()) {
        this->role_ = sharpen::RaftRole::Learner;
    }
}

sharpen::RaftConsensus::RaftConsensus(
    const sharpen::ActorId &id,
    std::unique_ptr<sharpen::IStatusMap> statusMap,
    std::unique_ptr<sharpen::ILogStorage> logs,
    std::unique_ptr<sharpen::IRaftLogAccesser> logAccesser,
    std::unique_ptr<sharpen::IRaftSnapshotController> snapshotController,
    std::shared_ptr<sharpen::RaftLeaderCounter> leaderCount,
    const sharpen::RaftOption &option)
    : Self{id,
           std::move(statusMap),
           std::move(logs),
           std::move(logAccesser),
           std::move(snapshotController),
           std::move(leaderCount),
           option,
           sharpen::GetLocalScheduler()} {
}

sharpen::RaftConsensus::RaftConsensus(
    const sharpen::ActorId &id,
    std::unique_ptr<sharpen::IStatusMap> statusMap,
    std::unique_ptr<sharpen::ILogStorage> logs,
    std::unique_ptr<sharpen::IRaftLogAccesser> logAccesser,
    std::unique_ptr<sharpen::IRaftSnapshotController> snapshotController,
    const sharpen::RaftOption &option)
    : Self{id,
           std::move(statusMap),
           std::move(logs),
           std::move(logAccesser),
           std::move(snapshotController),
           nullptr,
           option,
           sharpen::GetLocalScheduler()} {
}

void sharpen::RaftConsensus::DoNotifyWaiterWhenClose() noexcept {
    sharpen::Future<sharpen::ConsensusResult> *waiter{this->waiter_.exchange(nullptr)};
    if (waiter != nullptr) {
        this->NotifyWaiter(waiter);
    }
}

sharpen::RaftConsensus::~RaftConsensus() noexcept {
    // wait is unneeded
    // we will block when ~worker()
    this->worker_->Submit(&Self::DoNotifyWaiterWhenClose, this);
}

sharpen::Optional<std::uint64_t> sharpen::RaftConsensus::LoadUint64(sharpen::ByteSlice key) {
    assert(this->statusMap_ != nullptr);
    sharpen::ByteBuffer keyBuf{key};
    sharpen::Optional<sharpen::ByteBuffer> valOpt{this->statusMap_->Lookup(keyBuf)};
    if (valOpt.Exist()) {
        sharpen::ByteBuffer &val{valOpt.Get()};
        if (val.GetSize() == sizeof(std::uint64_t)) {
            std::uint64_t v{val.As<std::uint64_t>()};
#ifdef SHARPEN_IS_BIG_ENDIAN
            sharpen::ConvertEndian(v);
#endif
            return v;
        }
    }
    return sharpen::EmptyOpt;
}

void sharpen::RaftConsensus::LoadTerm() {
    sharpen::Optional<std::uint64_t> termOpt{this->LoadUint64(termKey)};
    if (termOpt.Exist()) {
        this->term_ = termOpt.Get();
    }
}

void sharpen::RaftConsensus::LoadVoteFor() {
    assert(this->statusMap_ != nullptr);
    sharpen::ByteBuffer key{voteKey};
    sharpen::Optional<sharpen::ByteBuffer> valOpt{this->statusMap_->Lookup(key)};
    if (valOpt.Exist()) {
        sharpen::ByteBuffer &val{valOpt.Get()};
        sharpen::BufferReader reader{val};
        sharpen::RaftVoteRecord vote;
        try {
            reader.Read(vote);
            this->vote_ = vote;
        } catch (const std::bad_alloc &fault) {
            std::terminate();
            (void)fault;
        } catch (const std::system_error &error) {
            sharpen::ErrorCode code{sharpen::GetErrorCode(error)};
            if (sharpen::IsFatalError(code)) {
                std::terminate();
            }
            (void)error;
        } catch (const std::exception &ignore) { (void)ignore; }
    }
}

sharpen::IRaftSnapshotInstaller &sharpen::RaftConsensus::GetSnapshotInstaller() noexcept {
    assert(this->snapshotController_ != nullptr);
    return this->snapshotController_->Installer();
}

const sharpen::IRaftSnapshotInstaller &
sharpen::RaftConsensus::GetSnapshotInstaller() const noexcept {
    assert(this->snapshotController_ != nullptr);
    return this->snapshotController_->Installer();
}

sharpen::IRaftSnapshotProvider &sharpen::RaftConsensus::GetSnapshotProvider() noexcept {
    assert(this->snapshotController_ != nullptr);
    return this->snapshotController_->Provider();
}

const sharpen::IRaftSnapshotProvider &sharpen::RaftConsensus::GetSnapshotProvider() const noexcept {
    assert(this->snapshotController_ != nullptr);
    return this->snapshotController_->Provider();
}

void sharpen::RaftConsensus::LoadCommitIndex() {
    std::uint64_t commitIndex{sharpen::ILogStorage::noneIndex};
    sharpen::Optional<std::uint64_t> lastAppiled{this->LoadUint64(lastAppiledKey)};
    if (lastAppiled.Exist()) {
        commitIndex = lastAppiled.Get();
    }
    if (this->snapshotController_) {
        sharpen::Optional<sharpen::RaftSnapshotMetadata> metadataOpt{
            this->GetSnapshotInstaller().GetLastMetadata()};
        if (metadataOpt.Exist()) {
            commitIndex = (std::max)(metadataOpt.Get().GetLastIndex(), commitIndex);
        }
    }
    this->SetCommitIndex(commitIndex);
}

void sharpen::RaftConsensus::SetUint64(sharpen::ByteSlice key, std::uint64_t value) {
    assert(this->statusMap_ != nullptr);
    if (key) {
        sharpen::ByteBuffer keyBuf{key};
        sharpen::ByteBuffer val{sizeof(value)};
        val.As<std::uint64_t>() = value;
        this->statusMap_->Write(keyBuf, val);
    }
}

std::uint64_t sharpen::RaftConsensus::GetTerm() const noexcept {
    return this->term_;
}

void sharpen::RaftConsensus::SetTerm(std::uint64_t term) {
    this->SetUint64(termKey, term);
    this->term_ = term;
}

std::uint64_t sharpen::RaftConsensus::GetCommitIndex() const noexcept {
    assert(this->heartbeatProvider_ != nullptr);
    return this->heartbeatProvider_->GetCommitIndex();
}

void sharpen::RaftConsensus::SetCommitIndex(std::uint64_t index) noexcept {
    assert(this->heartbeatProvider_ != nullptr);
    return this->heartbeatProvider_->SetCommitIndex(index);
}

sharpen::RaftVoteRecord sharpen::RaftConsensus::GetVote() const noexcept {
    return this->vote_;
}

void sharpen::RaftConsensus::SetVote(sharpen::RaftVoteRecord vote) {
    assert(this->statusMap_ != nullptr);
    sharpen::ByteBuffer key{voteKey};
    sharpen::ByteBuffer val;
    sharpen::BufferWriter writer{val};
    writer.Write(vote);
    this->statusMap_->Write(key, val);
    this->vote_ = vote;
}

std::uint64_t sharpen::RaftConsensus::GetLastIndex() const {
    std::uint64_t snapshotIndex{sharpen::ILogStorage::noneIndex};
    if (this->snapshotController_) {
        sharpen::Optional<sharpen::RaftSnapshotMetadata> metadataOpt{
            this->GetSnapshotInstaller().GetLastMetadata()};
        if (metadataOpt.Exist()) {
            snapshotIndex = metadataOpt.Get().GetLastIndex();
        }
    }
    assert(this->logs_ != nullptr);
    return (std::max)(this->logs_->GetLastIndex(), snapshotIndex);
}

sharpen::Optional<std::uint64_t> sharpen::RaftConsensus::LookupTerm(std::uint64_t index) const {
    if (index == sharpen::ILogStorage::noneIndex) {
        return sharpen::EmptyOpt;
    }
    if (this->snapshotController_) {
        sharpen::Optional<sharpen::RaftSnapshotMetadata> metadataOpt{
            this->GetSnapshotInstaller().GetLastMetadata()};
        if (metadataOpt.Exist() && index == metadataOpt.Get().GetLastIndex()) {
            return index = metadataOpt.Get().GetLastTerm();
        }
    }
    assert(this->logs_ != nullptr);
    return this->LookupTermOfEntry(index);
}

bool sharpen::RaftConsensus::Writable() const {
    return this->role_ == sharpen::RaftRole::Leader;
}

bool sharpen::RaftConsensus::Changable() const {
    return this->Writable();
}

void sharpen::RaftConsensus::EnsureConfig() const {
    if (!this->mailBuilder_) {
        throw std::logic_error{"should set mail builder first"};
    }
    if (!this->mailExtractor_) {
        throw std::logic_error{"should set mail extractor first"};
    }
    if (!this->peers_) {
        throw std::logic_error{"should configurate quorum first"};
    }
}

void sharpen::RaftConsensus::EnsureHearbeatProvider() {
    if (!this->heartbeatProvider_) {
        sharpen::RaftHeartbeatMailProvider *provider{
            new (std::nothrow) sharpen::RaftHeartbeatMailProvider{this->id_,
                                                                  *this->mailBuilder_,
                                                                  *this->logs_,
                                                                  *this->logAccesser_,
                                                                  this->snapshotController_.get(),
                                                                  this->option_.GetBatchSize()}};
        if (!provider) {
            throw std::bad_alloc{};
        }
        this->heartbeatProvider_.reset(provider);
        // set commit index
        this->LoadCommitIndex();
    }
}

void sharpen::RaftConsensus::PrepareMailBuilder(
    std::unique_ptr<sharpen::IRaftMailBuilder> builder) noexcept {
    assert(builder != nullptr && this->mailBuilder_ == nullptr);
    this->mailBuilder_ = std::move(builder);
}

void sharpen::RaftConsensus::PrepareMailExtractor(
    std::unique_ptr<sharpen::IRaftMailExtractor> extractor) noexcept {
    assert(extractor != nullptr && this->mailExtractor_ == nullptr);
    this->mailExtractor_ = std::move(extractor);
}

bool sharpen::RaftConsensus::NviIsConsensusMail(const sharpen::Mail &mail) const noexcept {
    assert(this->mailExtractor_ != nullptr);
    return this->mailExtractor_->IsRaftMail(mail);
}

void sharpen::RaftConsensus::NviWaitNextConsensus(
    sharpen::Future<sharpen::ConsensusResult> &future) {
    assert(this->worker_ != nullptr);
    this->waiter_.store(&future);
    std::uint64_t advancedCount{this->advancedCount_.load()};
    std::uint64_t reachCount{this->reachAdvancedCount_.load()};
    if (advancedCount > reachCount) {
        sharpen::Future<sharpen::ConsensusResult> *waiter{this->waiter_.exchange(nullptr)};
        if (waiter) {
            this->reachAdvancedCount_.compare_exchange_strong(reachCount, advancedCount);
            sharpen::ConsensusResult result{this->lastResult_.Take()};
            waiter->Complete(result);
        }
    }
}

void sharpen::RaftConsensus::EnsureBroadcaster() {
    if (!this->peersBroadcaster_) {
        assert(this->peers_ != nullptr);
        this->peersBroadcaster_ =
            this->peers_->CreateBroadcaster(this->option_.GetPipelineLength());
    }
}

sharpen::Optional<std::uint64_t> sharpen::RaftConsensus::LookupTermOfEntry(
    std::uint64_t index) const noexcept {
    assert(index != sharpen::ILogStorage::noneIndex);
    assert(this->logs_ != nullptr);
    assert(this->logAccesser_ != nullptr);
    sharpen::Optional<sharpen::ByteBuffer> entryOpt{this->logs_->Lookup(index)};
    if (entryOpt.Exist()) {
        return this->logAccesser_->LookupTerm(entryOpt.Get());
    }
    return sharpen::EmptyOpt;
}

bool sharpen::RaftConsensus::CheckEntry(std::uint64_t index,
                                        std::uint64_t expectedTerm) const noexcept {
    assert(this->logs_ != nullptr);
    assert(index != sharpen::ILogStorage::noneIndex);
    sharpen::Optional<std::uint64_t> termOpt{this->LookupTermOfEntry(index)};
    if (termOpt.Exist()) {
        std::uint64_t term{termOpt.Get()};
        return term == expectedTerm;
    }
    return false;
}

sharpen::Mail sharpen::RaftConsensus::OnPrevoteRequest(const sharpen::RaftPrevoteRequest &request) {
    assert(this->mailBuilder_ != nullptr);
    sharpen::RaftPrevoteResponse response;
    response.SetStatus(false);
    response.SetTerm(this->GetTerm());
    // get last index and last term
    std::uint64_t lastIndex{this->GetLastIndex()};
    std::uint64_t lastTerm{sharpen::ConsensusWriter::noneEpoch};
    sharpen::Optional<std::uint64_t> lastTermOpt{this->LookupTerm(lastIndex)};
    assert((lastTermOpt.Exist() && lastIndex != sharpen::ILogStorage::noneIndex) ||
           (!lastTermOpt.Exist() && lastIndex == sharpen::ILogStorage::noneIndex));
    if (lastTermOpt.Exist()) {
        lastTerm = lastTermOpt.Get();
    }
    if (request.GetLastTerm() > lastTerm ||
        (request.GetLastIndex() >= lastIndex && request.GetLastTerm() >= lastTerm)) {
        response.SetStatus(true);
    }
    // return response
    sharpen::Mail mail{this->mailBuilder_->BuildPrevoteResponse(response)};
    return mail;
}

sharpen::Mail sharpen::RaftConsensus::OnVoteRequest(const sharpen::RaftVoteForRequest &request) {
    assert(this->mailBuilder_ != nullptr);
    sharpen::RaftVoteForResponse response;
    response.SetStatus(false);
    response.SetTerm(this->GetTerm());
    // if term >= current term
    if (this->peers_->Exist(request.ActorId()) && request.GetTerm() >= this->GetTerm()) {
        // set current term = term
        if (request.GetTerm() > this->GetTerm()) {
            std::uint64_t newTerm{request.GetTerm()};
            this->SetTerm(newTerm);
            response.SetTerm(newTerm);
            this->Abdicate();
        }
        // check vote record
        sharpen::RaftVoteRecord vote{this->GetVote()};
        // if already vote in current term
        if (vote.GetTerm() == this->GetTerm()) {
            assert(vote.GetTerm() != sharpen::ConsensusWriter::noneEpoch);
            // set true if voteid == id
            if (vote.ActorId() == request.ActorId()) {
                response.SetStatus(true);
            }
        } else {
            // get last index and last term
            std::uint64_t lastIndex{this->GetLastIndex()};
            std::uint64_t lastTerm{sharpen::ConsensusWriter::noneEpoch};
            sharpen::Optional<std::uint64_t> lastTermOpt{this->LookupTerm(lastIndex)};
            assert((lastTermOpt.Exist() && lastIndex != sharpen::ILogStorage::noneIndex) ||
                   (!lastTermOpt.Exist() && lastIndex == sharpen::ILogStorage::noneIndex));
            if (lastTermOpt.Exist()) {
                lastTerm = lastTermOpt.Get();
            }
            // set true if logs up-to-date current logs
            if (request.GetLastTerm() > lastTerm ||
                (request.GetLastIndex() >= lastIndex && request.GetLastTerm() >= lastTerm)) {
                bool win{true};
                if (this->leaderCount_ != nullptr && request.GetLastTerm() == lastTerm &&
                    request.GetLastIndex() == lastIndex) {
                    if (this->leaderCount_->GetCurrentCount() < request.GetLeaderCount()) {
                        win = false;
                    }
                }
                if (win) {
                    // save vote record
                    vote.ActorId() = request.ActorId();
                    vote.SetTerm(this->GetTerm());
                    this->SetVote(vote);
                    // set status to true
                    response.SetStatus(true);
                }
            }
        }
    }
    // return response
    sharpen::Mail mail{this->mailBuilder_->BuildVoteResponse(response)};
    return mail;
}

sharpen::Mail sharpen::RaftConsensus::OnHeartbeatRequest(
    const sharpen::RaftHeartbeatRequest &request) {
    assert(this->mailBuilder_ != nullptr);
    assert(this->logs_ != nullptr);
    assert(this->logAccesser_ != nullptr);
    sharpen::RaftHeartbeatResponse response;
    response.SetStatus(false);
    response.SetTerm(this->GetTerm());
    response.SetLeaseRound(request.GetLeaseRound());
    // check leader id
    sharpen::ConsensusWriter leaderRecord{this->leaderRecord_.GetRecord()};
    if (this->peers_->Exist(request.LeaderActorId()) && request.GetTerm() >= this->GetTerm() &&
        (leaderRecord.GetEpoch() != request.GetTerm() ||
         leaderRecord.WriterId() == request.LeaderActorId())) {
        // if term > current term
        if (request.GetTerm() > this->GetTerm()) {
            std::uint64_t newTerm{request.GetTerm()};
            this->SetTerm(newTerm);
            response.SetTerm(newTerm);
            this->Abdicate();
        }
        bool validatedEntries{true};
        // check request entires
        for (std::size_t i = 0; i != request.Entries().GetSize(); ++i) {
            if (!this->logAccesser_->IsRaftEntry(request.Entries().Get(i))) {
                validatedEntries = false;
                break;
            }
        }
        std::uint64_t lastIndex{this->GetLastIndex()};
        std::uint64_t commitIndex{this->GetCommitIndex()};
        // last index must >= pre index, so we could check logs
        // commit index must <= pre index, so we make sure this request is validated
        if (validatedEntries && lastIndex >= request.GetPreLogIndex() &&
            request.GetPreLogIndex() >= commitIndex && this->role_ != sharpen::RaftRole::Leader) {
            // check pre index & pre term
            sharpen::Optional<std::uint64_t> termOpt{sharpen::EmptyOpt};
            if (request.GetPreLogIndex() != sharpen::ILogStorage::noneIndex) {
                termOpt = this->LookupTerm(request.GetPreLogIndex());
            } else {
                std::uint64_t tmp{sharpen::ConsensusWriter::noneEpoch};
                termOpt.Construct(tmp);
            }
            // if we find the log
            // check term of log
            if (termOpt.Exist() && termOpt.Get() == request.GetPreLogTerm()) {
                // if last index bigger than the last index of leader
                // truncate the logs, make sure our logs shorter than leader
                bool check{true};
                if (lastIndex > request.GetPreLogIndex() + request.Entries().GetSize()) {
                    this->logs_->TruncateFrom(request.GetPreLogIndex() + 1);
                    check = false;
                }
                // write log to logs and fix conflicts
                std::size_t offset{0};
                if (check) {
                    for (std::size_t i = 0; i != request.Entries().GetSize(); ++i) {
                        std::uint64_t index{request.GetPreLogIndex() + 1 + i};
                        std::uint64_t entryTerm{
                            this->logAccesser_->GetTerm(request.Entries().Get(i))};
                        if (!this->CheckEntry(index, entryTerm)) {
                            this->logs_->TruncateFrom(index);
                            break;
                        }
                        offset = i;
                    }
                }
                std::uint64_t matchIndex{request.GetPreLogIndex() + request.Entries().GetSize()};
                sharpen::LogEntries entires{request.Entries(), offset};
                this->logs_->WriteBatch(request.GetPreLogIndex() + 1 + offset, std::move(entires));
                response.SetStatus(true);
                // set match index to last index
                response.SetMatchIndex(matchIndex);
                // flush leader id if we need
                if (leaderRecord.GetEpoch() < request.GetTerm()) {
                    this->leaderRecord_.Flush(request.GetTerm(), request.LeaderActorId());
                }
                // set commit index
                this->SetCommitIndex(request.GetCommitIndex());
            }
        }
        // always advance state machine
        if (commitIndex != this->GetCommitIndex()) {
            this->OnStatusChanged({sharpen::ConsensusResultEnum::LogsCommit,
                                   sharpen::ConsensusResultEnum::LeaseRequested});
        } else {
            this->OnStatusChanged({sharpen::ConsensusResultEnum::LeaseRequested});
        }
        if (!response.GetStatus()) {
            // if we failure in some way
            // set match index to current commit index
            // then leader could relocate conflict point
            response.SetMatchIndex(this->GetCommitIndex());
        }
    }
    sharpen::Mail mail{this->mailBuilder_->BuildHeartbeatResponse(response)};
    return mail;
}

sharpen::Mail sharpen::RaftConsensus::OnSnapshotRequest(
    const sharpen::RaftSnapshotRequest &request) {
    assert(this->mailBuilder_ != nullptr);
    assert(this->logs_ != nullptr);
    sharpen::RaftSnapshotResponse response;
    response.SetTerm(this->GetTerm());
    response.SetStatus(false);
    response.SetLeaseRound(request.GetLeaseRound());
    // check leader id
    sharpen::ConsensusWriter leaderRecord{this->leaderRecord_.GetRecord()};
    if (this->peers_->Exist(request.LeaderActorId()) && request.GetTerm() >= this->GetTerm() &&
        (leaderRecord.GetEpoch() != request.GetTerm() ||
         leaderRecord.WriterId() == request.LeaderActorId())) {
        // if term > current term
        if (request.GetTerm() > this->GetTerm()) {
            // set new term
            std::uint64_t newTerm{request.GetTerm()};
            this->SetTerm(newTerm);
            response.SetTerm(newTerm);
            this->Abdicate();
        }
        // if enable snapshot
        // request.term is up-to-date
        // check offset
        if (this->snapshotController_ && this->GetTerm() == request.GetTerm() &&
            this->role_ != sharpen::RaftRole::Leader &&
            this->GetSnapshotInstaller().GetExpectedOffset() == request.GetOffset()) {
            // write snapshot chunk
            this->GetSnapshotInstaller().Write(request.GetOffset(), request.Data());
            if (request.IsLast()) {
                // if this chunk is last one
                // install snapshot(could be asynchronous)
                this->GetSnapshotInstaller().Install(request.Metadata());
                // set commit index to last index of snapshot
                this->SetCommitIndex(request.Metadata().GetLastIndex());
            }
            response.SetStatus(true);
            // flush leader id if we need
            if (leaderRecord.GetEpoch() < request.GetTerm()) {
                this->leaderRecord_.Flush(request.GetTerm(), request.LeaderActorId());
            }
            this->OnStatusChanged({sharpen::ConsensusResultEnum::SnapshotReceived,
                                   sharpen::ConsensusResultEnum::LeaseRequested});
        }
    }
    sharpen::Mail mail{this->mailBuilder_->BuildSnapshotResponse(response)};
    return mail;
}

void sharpen::RaftConsensus::NotifyWaiter(
    sharpen::Future<sharpen::ConsensusResult> *waiter) noexcept {
    assert(waiter != nullptr);
    try {
        sharpen::ConsensusResult result{this->lastResult_.Take()};
        waiter->Complete(result);
    } catch (const std::system_error &error) {
        sharpen::ErrorCode code{sharpen::GetErrorCode(error)};
        if (sharpen::IsFatalError(code)) {
            std::terminate();
        }
        (void)error;
    } catch (const std::bad_alloc &fault) {
        std::terminate();
        (void)fault;
    } catch (const std::exception &ignore) { (void)ignore; }
}

void sharpen::RaftConsensus::ComeToPower() {
    // adbicate by leader balance
    if (this->leaderCount_ != nullptr &&
        !this->leaderCount_->TryComeToPower(this->prevLeaderCount_)) {
        return;
    }
    this->role_ = sharpen::RaftRole::Leader;
    assert(this->heartbeatProvider_ != nullptr);
    this->OnStatusChanged({sharpen::ConsensusResultEnum::StatusChanged});
    // draining pipeline
    if (this->peersBroadcaster_) {
        this->peersBroadcaster_->Drain();
    }
}

void sharpen::RaftConsensus::Abdicate() {
    if (this->role_ != sharpen::RaftRole::Learner) {
        sharpen::RaftRole role{sharpen::RaftRole::Follower};
        role = this->role_.exchange(role);
        if (role == sharpen::RaftRole::Leader) {
            this->leaderCount_->Abdicate();
            this->OnStatusChanged({sharpen::ConsensusResultEnum::StatusChanged});
        }
    }
}

void sharpen::RaftConsensus::OnStatusChanged(
    std::initializer_list<sharpen::ConsensusResultEnum> results) {
    for (auto begin = results.begin(), end = results.end(); begin != end; ++begin) {
        this->lastResult_.Set(*begin);
    }
    std::uint64_t advancedCount{this->advancedCount_.fetch_add(1) + 1};
    sharpen::Future<sharpen::ConsensusResult> *future{this->waiter_.exchange(nullptr)};
    if (future) {
        this->reachAdvancedCount_.store(advancedCount);
        this->NotifyWaiter(future);
    }
}

void sharpen::RaftConsensus::OnPrevoteResponse(const sharpen::RaftPrevoteResponse &response,
                                               const sharpen::ActorId &actorId) {
    std::uint64_t term{this->GetTerm()};
    // if term > current term
    if (response.GetTerm() > term) {
        assert(!response.GetStatus());
        this->SetTerm(response.GetTerm());
        return this->Abdicate();
    }
    if (response.GetStatus()) {
        // make sure the term that we raise prevote
        // equals with current term
        if (term == this->prevoteRecord_.GetTerm()) {
            assert(response.GetTerm() <= term);
            this->prevoteRecord_.Receive(actorId);
            // if we got a quorum
            // raise election
            if (this->prevoteRecord_.GetVotes() == this->peers_->GetMajority()) {
                this->RaiseElection();
            }
        }
    }
}

void sharpen::RaftConsensus::OnVoteResponse(const sharpen::RaftVoteForResponse &response,
                                            const sharpen::ActorId &actorId) {
    assert(this->peers_);
    assert(this->peers_->Exist(actorId));
    if (response.GetTerm() > this->GetTerm()) {
        assert(!response.GetStatus());
        this->SetTerm(response.GetTerm());
        return this->Abdicate();
    }
    if (!response.GetStatus()) {
        return;
    }
    std::uint64_t electionTerm{this->electionRecord_.GetTerm()};
    // check term
    if (electionTerm == response.GetTerm() && electionTerm == this->GetTerm()) {
        std::uint64_t votes{this->electionRecord_.GetVotes()};
        votes += 1;
        this->electionRecord_.SetVotes(votes);
        // check if we could be leader
        if (votes == this->peers_->GetMajority()) {
            this->ComeToPower();
        }
    }
}

void sharpen::RaftConsensus::OnHeartbeatResponse(const sharpen::RaftHeartbeatResponse &response,
                                                 const sharpen::ActorId &actorId) {
    assert(this->heartbeatProvider_ != nullptr);
    assert(!this->heartbeatProvider_->Empty());
    // if term > current term
    // leader abdicate
    if (response.GetTerm() > this->GetTerm()) {
        assert(!response.GetStatus());
        this->SetTerm(response.GetTerm());
        this->Abdicate();
    }
    if (response.GetStatus() && response.GetTerm() == this->GetTerm()) {
        bool leaseConfirmed{false};
        if (this->option_.IsEnableLeaseAwareness() &&
            response.GetLeaseRound() == this->leaseStatus_.GetRound()) {
            this->leaseStatus_.OnAck();
            if (this->leaseStatus_.GetAckCount() == this->peers_->GetMajority()) {
                leaseConfirmed = true;
            }
        }
        // load current commit index
        std::uint64_t commitIndex{this->GetCommitIndex()};
        // foward replicated state and recompute commit index
        this->heartbeatProvider_->ForwardState(actorId, response.GetMatchIndex());
        // if the commit index forward
        // notify waiter
        if (commitIndex != this->GetCommitIndex()) {
            if (leaseConfirmed) {
                this->OnStatusChanged({sharpen::ConsensusResultEnum::LogsCommit,
                                       sharpen::ConsensusResultEnum::LeaseConfirmed});
            } else {
                this->OnStatusChanged({sharpen::ConsensusResultEnum::LogsCommit});
            }
        } else if (leaseConfirmed) {
            this->OnStatusChanged({sharpen::ConsensusResultEnum::LeaseConfirmed});
        }
    } else {
        // if we failure
        // backward replicated state
        // if match index of response > our match index
        // do nothing, pipeline will reach it at the end
        this->heartbeatProvider_->BackwardState(actorId, response.GetMatchIndex());
    }
}

void sharpen::RaftConsensus::OnSnapshotResponse(const sharpen::RaftSnapshotResponse &response,
                                                const sharpen::ActorId &actorId) {
    assert(this->heartbeatProvider_ != nullptr);
    if (response.GetTerm() > this->GetTerm()) {
        assert(!response.GetStatus());
        this->SetTerm(response.GetTerm());
        this->Abdicate();
    } else if (this->option_.IsEnableLeaseAwareness() &&
               response.GetLeaseRound() == this->leaseStatus_.GetRound()) {
        this->leaseStatus_.OnAck();
        if (this->leaseStatus_.GetAckCount() == this->peers_->GetMajority()) {
            this->OnStatusChanged({sharpen::ConsensusResultEnum::LeaseConfirmed});
        }
    }
    // nothing we can do
    (void)response;
    (void)actorId;
}

sharpen::Mail sharpen::RaftConsensus::DoGenerateResponse(sharpen::Mail request) {
    assert(this->mailExtractor_ != nullptr);
    this->EnsureHearbeatProvider();
    sharpen::Mail response;
    sharpen::RaftMailType type{this->mailExtractor_->GetMailType(request)};
    switch (type) {
    case sharpen::RaftMailType::Unknown:
        break;
    case sharpen::RaftMailType::PrevoteRequest: {
        sharpen::Optional<sharpen::RaftPrevoteRequest> requestOpt{
            this->mailExtractor_->ExtractPrevoteRequest(request)};
        if (requestOpt.Exist()) {
            response = this->OnPrevoteRequest(requestOpt.Get());
        }
    } break;
    case sharpen::RaftMailType::VoteRequest: {
        sharpen::Optional<sharpen::RaftVoteForRequest> requestOpt{
            this->mailExtractor_->ExtractVoteRequest(request)};
        if (requestOpt.Exist()) {
            response = this->OnVoteRequest(requestOpt.Get());
        }
    } break;
    case sharpen::RaftMailType::HeartbeatRequest: {
        sharpen::Optional<sharpen::RaftHeartbeatRequest> requestOpt{
            this->mailExtractor_->ExtractHeartbeatRequest(request)};
        if (requestOpt.Exist()) {
            response = this->OnHeartbeatRequest(requestOpt.Get());
        }
    } break;
    case sharpen::RaftMailType::InstallSnapshotRequest: {
        sharpen::Optional<sharpen::RaftSnapshotRequest> requestOpt{
            this->mailExtractor_->ExtractSnapshotRequest(request)};
        if (requestOpt.Exist()) {
            response = this->OnSnapshotRequest(requestOpt.Get());
        }
    } break;
    default:
        // do nothing
        break;
    }
    return response;
}

void sharpen::RaftConsensus::DoReceive(sharpen::Mail mail, sharpen::ActorId actorId) {
    assert(this->mailExtractor_ != nullptr);
    sharpen::RaftMailType type{this->mailExtractor_->GetMailType(mail)};
    switch (type) {
    case sharpen::RaftMailType::PrevoteResponse: {
        sharpen::Optional<sharpen::RaftPrevoteResponse> responseOpt{
            this->mailExtractor_->ExtractPrevoteResponse(mail)};
        if (responseOpt.Exist()) {
            this->OnPrevoteResponse(responseOpt.Get(), actorId);
        }
    } break;
    case sharpen::RaftMailType::VoteResponse: {
        sharpen::Optional<sharpen::RaftVoteForResponse> responseOpt{
            this->mailExtractor_->ExtractVoteResponse(mail)};
        if (responseOpt.Exist()) {
            this->OnVoteResponse(responseOpt.Get(), actorId);
        }
    } break;
    case sharpen::RaftMailType::HeartbeatResponse: {
        sharpen::Optional<sharpen::RaftHeartbeatResponse> responseOpt{
            this->mailExtractor_->ExtractHeartbeatResponse(mail)};
        if (responseOpt.Exist()) {
            this->OnHeartbeatResponse(responseOpt.Get(), actorId);
        }
    } break;
    case sharpen::RaftMailType::InstallSnapshotResponse: {
        sharpen::Optional<sharpen::RaftSnapshotResponse> responseOpt{
            this->mailExtractor_->ExtractSnapshotResponse(mail)};
        if (responseOpt.Exist()) {
            this->OnSnapshotResponse(responseOpt.Get(), actorId);
        }
    } break;
    default:
        // do nothing
        break;
    }
}

void sharpen::RaftConsensus::NviReceive(sharpen::Mail mail, const sharpen::ActorId &actorId) {
    assert(this->worker_ != nullptr);
    if (this->IsConsensusMail(mail)) {
        this->worker_->Submit(&Self::DoReceive, this, std::move(mail), actorId);
    }
}

sharpen::Mail sharpen::RaftConsensus::NviGenerateResponse(sharpen::Mail request) {
    assert(this->worker_ != nullptr);
    sharpen::AwaitableFuture<sharpen::Mail> future;
    this->worker_->Invoke(future, &Self::DoGenerateResponse, this, std::move(request));
    return future.Await();
}

void sharpen::RaftConsensus::RaisePrevote() {
    assert(this->mailBuilder_ != nullptr);
    assert(this->peersBroadcaster_ != nullptr);
    assert(this->logs_ != nullptr);
    // load last index
    std::uint64_t lastIndex{this->GetLastIndex()};
    // load last term
    sharpen::Optional<std::uint64_t> lastTermOpt{this->LookupTerm(lastIndex)};
    assert((lastTermOpt.Exist() && lastIndex != sharpen::ILogStorage::noneIndex) ||
           (!lastTermOpt.Exist() && lastIndex == sharpen::ILogStorage::noneIndex));
    std::uint64_t lastTerm{sharpen::ConsensusWriter::noneEpoch};
    if (lastTermOpt.Exist()) {
        lastTerm = lastTermOpt.Get();
    }
    sharpen::RaftPrevoteRequest request;
    request.SetLastIndex(lastIndex);
    request.SetLastTerm(lastTerm);
    std::uint64_t term{this->GetTerm()};
    this->prevoteRecord_.Flush(term);
    sharpen::Mail mail{this->mailBuilder_->BuildPrevoteRequest(request)};
    this->peersBroadcaster_->Broadcast(std::move(mail));
}

void sharpen::RaftConsensus::RaiseElection() {
    assert(this->mailBuilder_ != nullptr);
    assert(this->peersBroadcaster_ != nullptr);
    assert(this->logs_ != nullptr);
    // get current term
    std::uint64_t term{this->GetTerm()};
    term += 1;
    this->SetTerm(term);
    // flush election record
    this->electionRecord_.Flush(term);
    // vote to self
    sharpen::RaftVoteRecord vote{this->GetVote()};
    assert(vote.GetTerm() < term);
    vote.SetTerm(term);
    vote.ActorId() = this->id_;
    this->SetVote(vote);
    // build vote request
    sharpen::RaftVoteForRequest request;
    request.ActorId() = this->id_;
    // load last index
    std::uint64_t lastIndex{this->GetLastIndex()};
    // load last term
    sharpen::Optional<std::uint64_t> lastTermOpt{this->LookupTerm(lastIndex)};
    assert((lastTermOpt.Exist() && lastIndex != sharpen::ILogStorage::noneIndex) ||
           (!lastTermOpt.Exist() && lastIndex == sharpen::ILogStorage::noneIndex));
    std::uint64_t lastTerm{sharpen::ConsensusWriter::noneEpoch};
    if (lastTermOpt.Exist()) {
        lastTerm = lastTermOpt.Get();
    }
    // load leader count
    if (this->leaderCount_ != nullptr) {
        this->prevLeaderCount_ = this->leaderCount_->GetCurrentCount();
    }
    request.SetLastIndex(lastIndex);
    request.SetLastTerm(lastTerm);
    request.SetTerm(term);
    request.SetLeaderCount(this->prevLeaderCount_);
    // broadcast vote request
    sharpen::Mail mail{this->mailBuilder_->BuildVoteRequest(request)};
    this->peersBroadcaster_->Broadcast(std::move(mail));
}

void sharpen::RaftConsensus::DoAdvance() {
    assert(this->mailBuilder_ != nullptr);
    // ensure broadcaster
    this->EnsureBroadcaster();
    this->EnsureHearbeatProvider();
    switch (this->role_.load()) {
    case sharpen::RaftRole::Leader: {
        this->DoSyncHeartbeatProvider();
        this->heartbeatProvider_->PrepareTerm(this->GetTerm());
        if (this->option_.IsEnableLeaseAwareness()) {
            this->leaseStatus_.NextRound();
            this->heartbeatProvider_->PrepareRound(this->leaseStatus_.GetRound());
        }
        if (!this->heartbeatProvider_->Empty()) {
            sharpen::Optional<std::uint64_t> syncIndex{
                this->heartbeatProvider_->GetSynchronizedIndex()};
            if (syncIndex.Exist()) {
                sharpen::Mail mail{this->heartbeatProvider_->ProvideSynchronizedMail()};
                this->peersBroadcaster_->Broadcast(std::move(mail));
            } else {
                this->peersBroadcaster_->Broadcast(*this->heartbeatProvider_);
            }
        } else if (this->option_.IsEnableSingle()) {
            std::uint64_t commitIndex{this->GetCommitIndex()};
            std::uint64_t lastIndex{this->GetLastIndex()};
            if (commitIndex != lastIndex) {
                this->OnStatusChanged({sharpen::ConsensusResultEnum::LogsCommit,
                                       sharpen::ConsensusResultEnum::LeaseConfirmed});
            } else {
                this->OnStatusChanged({sharpen::ConsensusResultEnum::LeaseConfirmed});
            }
        }
        break;
    }
    case sharpen::RaftRole::Follower: {
        if (!this->peers_->Empty()) {
            if (!this->option_.IsEnablePrevote()) {
                this->RaiseElection();
            } else {
                this->RaisePrevote();
            }
        } else if (this->option_.IsEnableSingle()) {
            // get current term
            std::uint64_t term{this->GetTerm()};
            term += 1;
            this->SetTerm(term);
            // flush election record
            this->electionRecord_.Flush(term);
            // vote to self
            sharpen::RaftVoteRecord vote{this->GetVote()};
            vote.SetTerm(term);
            vote.ActorId() = this->id_;
            this->SetVote(vote);
            // become leader
            this->ComeToPower();
        }
        break;
    }
    case sharpen::RaftRole::Learner:
        // do nothing
        break;
    default:
        // unkown role
        // do nothing
        break;
    }
}

void sharpen::RaftConsensus::Advance() {
    this->EnsureConfig();
    this->worker_->Submit(&Self::DoAdvance, this);
}

const sharpen::ILogStorage &sharpen::RaftConsensus::ImmutableLogs() const noexcept {
    return *this->logs_;
}

void sharpen::RaftConsensus::DoSyncHeartbeatProvider() {
    if (this->heartbeatProvider_ &&
        this->heartbeatProvider_->GetSize() != this->peers_->GetSize()) {
        this->heartbeatProvider_->Clear();
        std::set<sharpen::ActorId> set{this->peers_->GenerateActorsSet()};
        for (auto begin = set.begin(), end = set.end(); begin != end; ++begin) {
            this->heartbeatProvider_->Register(*begin);
        }
    }
}

void sharpen::RaftConsensus::DoConfiguratePeers(
    std::function<std::unique_ptr<sharpen::IQuorum>(sharpen::IQuorum *)> configurater) {
    std::unique_ptr<sharpen::IQuorum> quorum{std::move(this->peers_)};
    this->peers_ = configurater(quorum.release());
    assert(this->peers_ != nullptr);
    // close broadcaster
    if (this->peersBroadcaster_) {
        this->peersBroadcaster_.reset(nullptr);
    }
    this->DoSyncHeartbeatProvider();
}

void sharpen::RaftConsensus::NviConfiguratePeers(
    std::function<std::unique_ptr<sharpen::IQuorum>(sharpen::IQuorum *)> configurater) {
    sharpen::AwaitableFuture<void> future;
    this->worker_->Invoke(future, &Self::DoConfiguratePeers, this, std::move(configurater));
    future.Await();
}

void sharpen::RaftConsensus::DoReleasePeers() {
    if (this->peersBroadcaster_) {
        this->peersBroadcaster_->Close();
    }
}

void sharpen::RaftConsensus::ReleasePeers() {
    assert(this->worker_ != nullptr);
    this->worker_->Submit(&Self::DoReleasePeers, this);
}

void sharpen::RaftConsensus::NviDropLogsUntil(std::uint64_t index) {
    index = (std::min)(index, this->GetCommitIndex());
    this->worker_->Submit(&sharpen::ILogStorage::DropUntil, this->logs_.get(), index);
}

sharpen::WriteLogsResult sharpen::RaftConsensus::NviWrite(const sharpen::LogBatch &logs) {
    assert(!logs.Empty());
    assert(this->worker_ != nullptr);
    sharpen::AwaitableFuture<sharpen::WriteLogsResult> future;
    this->worker_->Invoke(future, &Self::DoWrite, this, &logs);
    return future.Await();
}

sharpen::WriteLogsResult sharpen::RaftConsensus::DoWrite(const sharpen::LogBatch *logs) {
    assert(logs != nullptr);
    assert(!logs->Empty());
    assert(this->logAccesser_ != nullptr);
    std::uint64_t lastIndex{this->GetLastIndex()};
    if (!this->Writable()) {
        return sharpen::WriteLogsResult{lastIndex};
    }
    std::uint64_t beginIndex{lastIndex + 1};
    std::uint64_t term{this->GetTerm()};
    assert(term != sharpen::ConsensusWriter::noneEpoch);
    for (std::size_t i = 0; i != logs->GetSize(); ++i) {
        sharpen::ByteBuffer entry{this->logAccesser_->CreateEntry(logs->Get(i), term)};
        this->logs_->Write(beginIndex + i, entry);
    }
    lastIndex += logs->GetSize();
    return sharpen::WriteLogsResult{beginIndex, lastIndex};
}

sharpen::ConsensusWriter sharpen::RaftConsensus::GetWriterId() const noexcept {
    sharpen::ConsensusWriter record{this->leaderRecord_.GetRecord()};
    return record;
}

std::uint64_t sharpen::RaftConsensus::GetEpoch() const noexcept {
    return this->GetTerm();
}

void sharpen::RaftConsensus::DoStoreLastAppiledIndex(std::uint64_t index) {
    assert(this->statusMap_ != nullptr);
    this->EnsureHearbeatProvider();
    if (this->GetCommitIndex() > index) {
#ifdef SHARPEN_IS_BIG_ENDIAN
        sharpen::ConvertEndian(index);
#endif
        sharpen::ByteBuffer val{sizeof(index)};
        val.As<std::uint64_t>() = index;
        sharpen::ByteBuffer key{lastAppiledKey};
        this->statusMap_->Write(std::move(key), std::move(val));
    }
}

void sharpen::RaftConsensus::StoreLastAppiledIndex(std::uint64_t index) {
    assert(this->worker_ != nullptr);
    sharpen::AwaitableFuture<void> future;
    this->worker_->Invoke(future, &Self::DoStoreLastAppiledIndex, this, index);
    future.Await();
}