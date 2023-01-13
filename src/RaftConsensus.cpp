#include <sharpen/RaftConsensus.hpp>

#include <cassert>

#include <sharpen/SingleWorkerGroup.hpp>
#include <sharpen/BufferWriter.hpp>
#include <sharpen/BufferReader.hpp>
#include <sharpen/SystemError.hpp>

sharpen::RaftConsensus::RaftConsensus(std::uint64_t id,std::unique_ptr<sharpen::IStatusMap> statusMap,std::unique_ptr<sharpen::ILogStorage> logs,bool isLearner,sharpen::IFiberScheduler &scheduler)
    :scheduler_(&scheduler)
    ,id_(id)
    ,statusMap_(std::move(statusMap))
    ,logs_(std::move(logs))
    ,term_(0)
    ,commitIndex_(0)
    ,vote_(0,0)
    ,role_(sharpen::RaftRole::Follower)
    ,electionRecord_(0,0)
    ,waiters_()
    ,advancedCount_(0)
    ,mailBuilder_(nullptr)
    ,mailExtractor_(nullptr)
    ,quorum_(nullptr)
    ,quorumBroadcaster_(nullptr)
    ,worker_(nullptr)
{
    assert(this->id_ != 0);
    assert(this->statusMap_ != nullptr);
    assert(this->logs_ != nullptr);
    //set worker
    sharpen::IWorkerGroup *worker{new (std::nothrow) sharpen::SingleWorkerGroup{*this->scheduler_}};
    if(!worker)
    {
        throw std::bad_alloc{};
    }
    this->worker_.reset(worker);
    //load status cache
    this->LoadTerm();
    this->LoadCommitIndex();
    this->LoadVoteFor();
    if(isLearner)
    {
        this->role_ = sharpen::RaftRole::Learner;
    }
}

sharpen::RaftConsensus::RaftConsensus(std::uint64_t id,std::unique_ptr<sharpen::IStatusMap> statusMap,std::unique_ptr<sharpen::ILogStorage> logs,bool isLearner)
    :Self{id,std::move(statusMap),std::move(logs),isLearner,sharpen::GetLocalScheduler()}
{}

sharpen::Optional<std::uint64_t> sharpen::RaftConsensus::LoadUint64(sharpen::ByteSlice key)
{
    assert(this->statusMap_ != nullptr);
    sharpen::ByteBuffer keyBuf{key};
    sharpen::Optional<sharpen::ByteBuffer> valOpt{this->statusMap_->Lookup(keyBuf)};
    if(valOpt.Exist())
    {
        sharpen::ByteBuffer &val{valOpt.Get()};
        if(val.GetSize() == sizeof(std::uint64_t))
        {
            return val.As<std::uint64_t>();
        }
    }
    return sharpen::EmptyOpt;
}

void sharpen::RaftConsensus::LoadTerm()
{
    sharpen::Optional<std::uint64_t> termOpt{this->LoadUint64(termKey)};
    if(termOpt.Exist())
    {
        this->term_ = termOpt.Get();
    }
}

void sharpen::RaftConsensus::LoadVoteFor()
{
    assert(this->statusMap_ != nullptr);
    sharpen::ByteBuffer key{voteKey};
    sharpen::Optional<sharpen::ByteBuffer> valOpt{this->statusMap_->Lookup(key)};
    if(valOpt.Exist())
    {
        sharpen::ByteBuffer &val{valOpt.Get()};
        sharpen::BufferReader reader{val};
        sharpen::RaftVoteRecord vote;
        try
        {
            reader.Read(vote);
            this->vote_ = vote;
        }
        catch(const std::bad_alloc &fault)
        {
            std::terminate();
            (void)fault;
        }
        catch(const std::system_error &error)
        {
            sharpen::ErrorCode code{sharpen::GetErrorCode(error)};
            if(sharpen::IsFatalError(code))
            {
                std::terminate();
            }
            (void)error;
        }
        catch(const std::exception &ignore)
        {
            (void)ignore;
        }
    }
}

void sharpen::RaftConsensus::LoadCommitIndex()
{
    sharpen::Optional<std::uint64_t> lastAppiled{this->LoadUint64(lastAppiledKey)};
    if(lastAppiled.Exist())
    {
        this->commitIndex_ = lastAppiled.Get();
    }
}

void sharpen::RaftConsensus::SetUint64(sharpen::ByteSlice key,std::uint64_t value)
{
    assert(this->statusMap_ != nullptr);
    if(key)
    {
        sharpen::ByteBuffer keyBuf{key};
        sharpen::ByteBuffer val{sizeof(value)};
        val.As<std::uint64_t>() = value;
        this->statusMap_->Write(keyBuf,val);
    }
}

std::uint64_t sharpen::RaftConsensus::GetTerm() const noexcept
{
    return this->term_;    
}

void sharpen::RaftConsensus::SetTerm(std::uint64_t term)
{
    this->SetUint64(termKey,term);
    this->term_ = term;
}

std::uint64_t sharpen::RaftConsensus::GetId() const noexcept
{
    return this->id_;
}

sharpen::RaftVoteRecord sharpen::RaftConsensus::GetVote() const noexcept
{
    return this->vote_;
}

void sharpen::RaftConsensus::SetVote(sharpen::RaftVoteRecord vote)
{
    assert(this->statusMap_ != nullptr);
    sharpen::ByteBuffer key{voteKey};
    sharpen::ByteBuffer val;
    sharpen::BufferWriter writer{val};
    writer.Write(vote);
    this->statusMap_->Write(key,val);
    this->vote_ = vote;
}

std::uint64_t sharpen::RaftConsensus::GetCommitIndex() const noexcept
{
    return this->commitIndex_;
}

bool sharpen::RaftConsensus::Writable() const
{
    return this->role_ == sharpen::RaftRole::Leader;
}

bool sharpen::RaftConsensus::Changable() const
{
    return this->Writable();
}

void sharpen::RaftConsensus::EnsureConfig() const
{
    if(!this->mailBuilder_)
    {
        throw std::logic_error{"should set mail builder first"};
    }
    if(!this->mailExtractor_)
    {
        throw std::logic_error{"should set mail extractor first"};
    }
    if(!this->quorum_)
    {
        throw std::logic_error{"should configurate quorum first"};
    }
}

void sharpen::RaftConsensus::SetMailBuilder(std::unique_ptr<sharpen::IRaftMailBuilder> builder) noexcept
{
    assert(builder != nullptr && this->mailBuilder_ == nullptr);
    this->mailBuilder_ = std::move(builder);
}

void sharpen::RaftConsensus::SetMailExtractor(std::unique_ptr<sharpen::IRaftMailExtractor> extractor) noexcept
{
    assert(extractor != nullptr && this->mailExtractor_ == nullptr);
    this->mailExtractor_ = std::move(extractor);
}

bool sharpen::RaftConsensus::NviIsConsensusMail(const sharpen::Mail &mail) const noexcept
{
    assert(this->mailExtractor_ != nullptr);
    return this->mailExtractor_->IsRaftMail(mail);
}

void sharpen::RaftConsensus::DoStatusChanged(std::uint64_t advancedCount,sharpen::ConsensusWaiter waiter)
{
    //load advanced count
    std::uint64_t currentCount{this->advancedCount_.load()};
    //if advanced while waiting
    //notify waiter
    if(currentCount != advancedCount)
    {
        this->NotifyWaiter(waiter);
    }
    else
    {
        this->waiters_.emplace(waiter);
    }
}

void sharpen::RaftConsensus::NviStatusChanged(sharpen::Future<std::uint64_t> &future,std::uint64_t minIndex)
{
    assert(this->worker_ != nullptr);
    //record current advanced count
    std::uint64_t advancedCount{this->advancedCount_.load()};
    //pack waiter
    sharpen::ConsensusWaiter waiter{minIndex,future};
    this->worker_->Submit(&Self::DoStatusChanged,this,advancedCount,waiter);
}

void sharpen::RaftConsensus::EnsureBroadcaster()
{
    if(!this->quorumBroadcaster_)
    {
        assert(this->quorum_ != nullptr);
        this->quorumBroadcaster_ = this->quorum_->CreateBroadcaster();
    }
}

sharpen::Mail sharpen::RaftConsensus::OnVoteRequest(const sharpen::RaftVoteForRequest &request)
{
    assert(this->mailBuilder_ != nullptr);
    sharpen::RaftVoteForResponse response;
    response.SetStatus(false);
    response.SetTerm(this->GetTerm());
    //if term >= current term
    if(this->quorum_->Exist(request.GetId()) && request.GetTerm() >= this->GetTerm())
    {
        //set current term = term
        if(request.GetTerm() > this->GetTerm())
        {
            std::uint64_t newTerm{request.GetTerm()};
            this->SetTerm(newTerm);
            response.SetTerm(newTerm);
        }
        //check vote record
        sharpen::RaftVoteRecord vote{this->GetVote()};
        //if already vote in current term
        if(vote.GetTerm() == this->GetTerm())
        {
            //set true if voteid == id
            if(vote.GetActorId() == request.GetId())
            {
                response.SetStatus(true);
            }
        }
        else
        {
            //get last index and last term
            std::uint64_t lastIndex{this->logs_->GetLastIndex()};
            std::uint64_t lastTerm{0};
            sharpen::Optional<std::uint64_t> lastTermOpt{this->logs_->LookupTerm(lastIndex)};
            assert((lastTermOpt.Exist() && lastIndex != 0) || (!lastTermOpt.Exist() && lastIndex == 0));
            if(lastTermOpt.Exist())
            {
                lastTerm = lastTermOpt.Get();
            }
            //set true if logs up-to-date current logs
            if((request.GetLastIndex() >= lastIndex) || (request.GetLastIndex() == lastIndex && request.GetTerm() >= lastTerm))
            {
                response.SetStatus(true);
            }
        }
    }
    //return response
    sharpen::Mail mail{this->mailBuilder_->BuildVoteResponse(response)};
    return mail;
}

void sharpen::RaftConsensus::NotifyWaiter(sharpen::ConsensusWaiter waiter) noexcept
{
    std::uint64_t index{this->GetCommitIndex()};
    try
    {
        waiter.Future().Complete(index);
    }
    catch(const std::system_error &error)
    {
        sharpen::ErrorCode code{sharpen::GetErrorCode(error)};
        if(sharpen::IsFatalError(code))
        {
            std::terminate();
        }
        (void)error;
    }
    catch(const std::bad_alloc &fault)
    {
        std::terminate();
        (void)fault;
    }
    catch(const std::exception &ignore)
    {
        (void)ignore;
    }
}

void sharpen::RaftConsensus::OnStatusChanged()
{
    //increase advanced count
    this->advancedCount_ += 1;
    //load current commit index
    std::uint64_t index{this->GetCommitIndex()};
    //check if waiter is non-empty
    //and there are waiter's index <= current commit index
    while (!this->waiters_.empty() && this->waiters_.top().GetIndex() <= index)
    {
        //get first waiter
        sharpen::ConsensusWaiter waiter{this->waiters_.top()};
        //pop from waiters
        this->waiters_.pop();
        //notify waiter
        this->NotifyWaiter(waiter);
    }
}

void sharpen::RaftConsensus::OnVoteResponse(const sharpen::RaftVoteForResponse &response,std::uint64_t actorId)
{
    (void)actorId;
    assert(this->quorum_);
    std::uint64_t electionTerm{this->electionRecord_.GetTerm()};
    //check term
    if(electionTerm == response.GetTerm())
    {
        std::uint64_t votes{this->electionRecord_.GetVotes()};
        votes += 1;
        this->electionRecord_.SetVotes(votes);
        //check if we could be leader
        if(votes == this->quorum_->GetSize()/2)
        {
            this->role_ = sharpen::RaftRole::Leader;
            this->OnStatusChanged();
        }
    }
}

sharpen::Mail sharpen::RaftConsensus::DoGenerateResponse(sharpen::Mail request)
{
    assert(this->mailExtractor_ != nullptr);
    sharpen::Mail mail;
    sharpen::RaftMailType type{this->mailExtractor_->GetMailType(request)};
    switch (type)
    {
   case sharpen::RaftMailType::Unknown:
        break;
    case sharpen::RaftMailType::PreVoteRequest:
        break;
    case sharpen::RaftMailType::VoteRequest:
        {
            sharpen::Optional<sharpen::RaftVoteForRequest> requestOpt{this->mailExtractor_->ExtractVoteRequest(mail)};
            if(requestOpt.Exist())
            {
                mail = this->OnVoteRequest(requestOpt.Get());
            }
        }
        break;
    case sharpen::RaftMailType::InstallSnapshotRequest:
        break;
    default:
        //do nothing
        break;
    }
    return mail;
}

void sharpen::RaftConsensus::DoReceive(sharpen::Mail mail,std::uint64_t actorId)
{
    assert(this->mailExtractor_ != nullptr);
    sharpen::RaftMailType type{this->mailExtractor_->GetMailType(mail)};
    //TODO
    switch (type)
    {
    case sharpen::RaftMailType::Unknown:
        //do nothing
        break;
    case sharpen::RaftMailType::PreVoteResponse:
        break;
    case sharpen::RaftMailType::VoteResponse:
        {
            sharpen::Optional<sharpen::RaftVoteForResponse> responseOpt{this->mailExtractor_->ExtractVoteResponse(mail)};
            if(responseOpt.Exist())
            {
                this->OnVoteResponse(responseOpt.Get(),actorId);
            }
        }
        break;
    case sharpen::RaftMailType::HeartbeatResponse:
        break;
    case sharpen::RaftMailType::InstallSnapshotResponse:
        break;
    default:
        //do nothing
        break;
    }
}

void sharpen::RaftConsensus::NviReceive(sharpen::Mail mail,std::uint64_t actorId)
{
    assert(this->worker_ != nullptr);
    if(this->IsConsensusMail(mail))
    {
        this->worker_->Submit(&Self::DoReceive,this,std::move(mail),actorId);
    }
}

sharpen::Mail sharpen::RaftConsensus::NviGenerateResponse(sharpen::Mail request)
{
    assert(this->worker_ != nullptr);
    sharpen::AwaitableFuture<sharpen::Mail> future;
    this->worker_->Invoke(future,&Self::DoGenerateResponse,this,std::move(request));
    return future.Await();
}

void sharpen::RaftConsensus::RaiseElection()
{
    assert(this->mailBuilder_ != nullptr);
    assert(this->quorumBroadcaster_ != nullptr);
    assert(this->logs_ != nullptr);
    //get current term
    std::uint64_t term{this->GetTerm()};
    term += 1;
    this->SetTerm(term);
    //flush election record
    this->electionRecord_.Flush(term);
    //vote to self
    sharpen::RaftVoteRecord vote{this->GetVote()};
    vote.SetTerm(term);
    vote.SetActorId(this->GetId());
    this->SetVote(vote);
    //build vote request
    sharpen::RaftVoteForRequest request;
    request.SetId(this->GetId());
    //load last index
    std::uint64_t lastIndex{this->logs_->GetLastIndex()};
    //load last term
    sharpen::Optional<std::uint64_t> lastTermOpt{this->logs_->LookupTerm(lastIndex)};
    assert((lastTermOpt.Exist() && lastIndex != 0) || (!lastTermOpt.Exist() && lastIndex == 0));
    std::uint64_t lastTerm{0};
    if(lastTermOpt.Exist())
    {
        lastTerm = lastTermOpt.Get();
    }
    request.SetLastIndex(lastIndex);
    request.SetLastTerm(lastTerm);
    request.SetTerm(term);
    //broadcast vote request
    sharpen::Mail mail{this->mailBuilder_->BuildVoteRequest(request)};
    this->quorumBroadcaster_->Broadcast(mail);
}

void sharpen::RaftConsensus::DoAdvance()
{
    assert(this->mailBuilder_ != nullptr);
    //ensure broadcaster
    this->EnsureBroadcaster();
    switch (this->role_.load())
    {
    case sharpen::RaftRole::Leader:
        //TODO:Hearbeat
        break;
    case sharpen::RaftRole::Follower:
        {
            this->RaiseElection();
        }
        break;
    case sharpen::RaftRole::Learner:
        //do nothing
        break;
    default:
        //unkown role
        std::terminate();
        break;
    }
}

void sharpen::RaftConsensus::Advance()
{
    this->EnsureConfig();
    this->worker_->Submit(&Self::DoAdvance,this);
}

const sharpen::ILogStorage &sharpen::RaftConsensus::ImmutableLogs() const noexcept
{
    return *this->logs_;
}

void sharpen::RaftConsensus::DoConfigurateQuorum(std::function<std::unique_ptr<sharpen::IQuorum>(sharpen::IQuorum*)> configurater)
{
    std::unique_ptr<sharpen::IQuorum> quorum{std::move(this->quorum_)};
    this->quorum_ = configurater(quorum.release());
    assert(this->quorum_ != nullptr);
    //release broadcaster
    if(this->quorumBroadcaster_)
    {
        this->quorumBroadcaster_.reset(nullptr);
    }
}

void sharpen::RaftConsensus::ConfigurateQuorum(std::function<std::unique_ptr<sharpen::IQuorum>(sharpen::IQuorum*)> configurater)
{
    sharpen::AwaitableFuture<void> future;
    this->worker_->Invoke(future,&Self::DoConfigurateQuorum,this,std::move(configurater));
    future.Await();
}

void sharpen::RaftConsensus::NviDropLogsUntil(std::uint64_t index)
{
    this->worker_->Submit(&sharpen::ILogStorage::DropUntil,this->logs_.get(),index);
}

std::uint64_t sharpen::RaftConsensus::NviWrite(std::unique_ptr<sharpen::ILogBatch> logs)
{
    assert(logs != nullptr);
    assert(this->worker_ != nullptr);
    sharpen::AwaitableFuture<std::uint64_t> future;
    this->worker_->Invoke(future,&Self::DoWrite,this,logs.release());
    return future.Await();
}

std::uint64_t sharpen::RaftConsensus::DoWrite(sharpen::ILogBatch *rawLogs)
{
    std::unique_ptr<sharpen::ILogBatch> logs{rawLogs};
    assert(logs != nullptr);
    std::uint64_t index{this->logs_->GetLastIndex()};
    for(std::size_t i = 0;i != logs->GetSize();++i)
    {
        this->logs_->Write(index + i,logs->Get(index));
    }
    index += logs->GetSize();
    return index;
}

std::unique_ptr<sharpen::ILogBatch> sharpen::RaftConsensus::CreateLogBatch() const
{
    //TODO 
    return nullptr;
}