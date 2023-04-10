#include <sharpen/RaftHeartbeatMailProvider.hpp>

sharpen::RaftHeartbeatMailProvider::RaftHeartbeatMailProvider(std::uint64_t id,const sharpen::IRaftMailBuilder &builder,const sharpen::ILogStorage &log,sharpen::IRaftSnapshotProvider *snapshotProvider)
    :Self{id,builder,log,snapshotProvider,defaultBatchSize_}
{}

sharpen::RaftHeartbeatMailProvider::RaftHeartbeatMailProvider(std::uint64_t id,const sharpen::IRaftMailBuilder &builder,const sharpen::ILogStorage &log,sharpen::IRaftSnapshotProvider *snapshotProvider,std::uint16_t batchSize)
    :id_(id)
    ,builder_(&builder)
    ,logs_(&log)
    ,snapshotProvider_(snapshotProvider)
    ,batchSize_(batchSize)
    ,states_()
    ,term_(0)
    ,commitIndex_(0)
{
    assert(this->batchSize_ >= 1);
}

sharpen::RaftHeartbeatMailProvider::RaftHeartbeatMailProvider(Self &&other) noexcept
    :id_(other.id_)
    ,builder_(other.builder_)
    ,logs_(other.logs_)
    ,snapshotProvider_(other.snapshotProvider_)
    ,batchSize_(other.batchSize_)
    ,states_(std::move(other.states_))
    ,term_(other.term_)
    ,commitIndex_(other.commitIndex_)
{
    other.id_ = 0;
    other.builder_ = nullptr;
    other.logs_ = nullptr;
    other.snapshotProvider_ = nullptr;
    other.batchSize_ = 0;
    other.term_ = 0;
    other.commitIndex_ = 0;
}

sharpen::RaftHeartbeatMailProvider &sharpen::RaftHeartbeatMailProvider::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->id_ = other.id_;
        this->builder_ = other.builder_;
        this->logs_ = other.logs_;
        this->snapshotProvider_ = other.snapshotProvider_;
        this->batchSize_ = other.batchSize_;
        this->states_ = std::move(other.states_);
        this->term_ = other.term_;
        this->commitIndex_ = other.commitIndex_;
        other.id_ = 0;
        other.builder_ = nullptr;
        other.logs_ = nullptr;
        other.snapshotProvider_ = nullptr;
        other.batchSize_ = 0;
        other.term_ = 0;
        other.commitIndex_ = 0;
    }
    return *this;
}

const sharpen::RaftReplicatedState *sharpen::RaftHeartbeatMailProvider::LookupState(std::uint64_t actorId) const noexcept
{
    auto ite = this->states_.find(actorId);
    if(ite != this->states_.end())
    {
        return &ite->second;
    }
    return nullptr;
}

sharpen::RaftReplicatedState *sharpen::RaftHeartbeatMailProvider::LookupMutableState(std::uint64_t actorId) const noexcept
{
    auto ite = this->states_.find(actorId);
    if(ite != this->states_.end())
    {
        return &ite->second;
    }
    return nullptr;
}

// void sharpen::RaftHeartbeatMailProvider::SetIndex(std::uint64_t actorId,std::uint64_t index)
// {
//     auto ite = this->nextIndexs_.find(actorId);
//     if(ite != this->nextIndexs_.end())
//     {
//         ite->second = index;
//     }
//     else
//     {
//         this->nextIndexs_.emplace(actorId,index);
//     }
// }

void sharpen::RaftHeartbeatMailProvider::RemoveState(std::uint64_t actorId) noexcept
{
    auto ite = this->states_.find(actorId);
    if(ite != this->states_.end())
    {
        this->states_.erase(ite);
    }
}

std::size_t sharpen::RaftHeartbeatMailProvider::GetSize() const noexcept
{
    return this->states_.size();
}

bool sharpen::RaftHeartbeatMailProvider::Empty() const noexcept
{
    return this->states_.empty();
}

sharpen::Optional<std::uint64_t> sharpen::RaftHeartbeatMailProvider::GetSynchronizedIndex() const noexcept
{
    if(!this->states_.empty())
    {
        auto begin = this->states_.begin();
        auto end = this->states_.end();
        std::uint64_t index{begin->second.GetNextIndex()};
        if(begin->second.LookupSnapshot())
        {
            return sharpen::EmptyOpt;
        }
        ++begin;
        while (begin != end)
        {
            if(begin->second.GetNextIndex() != index || begin->second.LookupSnapshot())
            {
                return sharpen::EmptyOpt;
            }
            ++begin;
        }
        return index;
    }
    return static_cast<std::uint64_t>(0);
}

std::uint64_t sharpen::RaftHeartbeatMailProvider::GetCommitIndex() const noexcept
{
    std::uint64_t index{this->commitIndex_};
    if(!this->states_.empty())
    {
        std::size_t majority{this->states_.size()};
        majority += 1;
        majority /= 2;
        //nested loop aggregation
        for(auto begin = this->states_.begin(),end = this->states_.end(); begin != end; ++begin)
        {
            std::uint64_t matchIndex{begin->second.GetMatchIndex()};
            if(matchIndex > index)
            {
                auto ite = begin;
                ++ite;
                std::size_t count{1};
                while(ite != end)
                {
                    if(ite->second.GetMatchIndex() >= matchIndex)
                    {
                        count += 1;
                    }
                    ++ite;
                }
                if(count >= majority)
                {
                    index = matchIndex;
                }
            }
        }
        this->commitIndex_ = (std::max)(index,this->commitIndex_);
    }
    return this->commitIndex_;
}

void sharpen::RaftHeartbeatMailProvider::SetCommitIndex(std::uint64_t index) noexcept
{
    this->commitIndex_ = (std::max)(index,this->GetCommitIndex());
}

void sharpen::RaftHeartbeatMailProvider::PrepareTerm(std::uint64_t term) noexcept
{
    this->term_ = term;
}

sharpen::Mail sharpen::RaftHeartbeatMailProvider::ProvideSnapshotRequest(sharpen::RaftReplicatedState *state) const
{
    assert(this->snapshotProvider_ != nullptr);
    if(!this->snapshotProvider_)
    {
        //snapshot already disable
        return sharpen::Mail{};
    }
    assert(state != nullptr);
    sharpen::IRaftSnapshotChunk *snapshot{state->LookupSnapshot()};
    sharpen::Optional<sharpen::RaftSnapshotMetadata> metadataOpt{state->LookupSnapshotMetadata()};
    assert(metadataOpt.Exist());
    assert(snapshot != nullptr);
    if(snapshot == nullptr || !metadataOpt.Exist())
    {
        return sharpen::Mail{};
    }
    sharpen::RaftSnapshotMetadata metadata{metadataOpt.Get()};
    sharpen::RaftSnapshotRequest request;
    request.SetTerm(this->term_);
    request.SetLeaderId(this->id_);
    request.SetOffset(snapshot->GetOffset());
    request.SetLast(!snapshot->Forwardable());
    request.Metadata() = metadata;
    request.Data() = snapshot->GenerateChunkData();
    return this->builder_->BuildSnapshotRequest(request);
}

sharpen::Mail sharpen::RaftHeartbeatMailProvider::Provide(std::uint64_t actorId) const
{
    assert(this->builder_);
    assert(this->logs_);
    assert(this->batchSize_ >= 1);
    //lookup next index
    sharpen::RaftReplicatedState *state{this->LookupMutableState(actorId)};
    if(!state)
    {
        return sharpen::Mail{};
    }
    if(state->LookupSnapshot())
    {
        return this->ProvideSnapshotRequest(state);
    }
    //get next index & match index
    std::uint64_t nextIndex{state->GetNextIndex()};
    std::uint64_t matchIndex{state->GetMatchIndex()};
    assert(matchIndex <= nextIndex);
    //get last index of current logs
    std::uint64_t lastIndex{this->logs_->GetLastIndex()};
    //compute pre index
    std::uint64_t preIndex{nextIndex};
    if(preIndex)
    {
        preIndex -= 1;
    }
    assert(nextIndex <= lastIndex);
    //compute logs size
    std::uint64_t size{lastIndex - nextIndex};
    //limit logs <= batchSize
    size = (std::min)(this->batchSize_,size);
    lastIndex = nextIndex + size;
    sharpen::RaftHeartbeatRequest request;
    //get commit index
    std::uint64_t commitIndex{(std::min)(this->GetCommitIndex(),lastIndex)};
    request.SetCommitIndex(commitIndex);
    request.SetLeaderId(this->id_);
    request.SetTerm(this->term_);
    request.SetPreLogIndex(preIndex);
    sharpen::Optional<std::uint64_t> term{this->logs_->LookupTerm(preIndex)};
    if(!term.Exist())
    {
        std::unique_ptr<sharpen::IRaftSnapshot> snapshot{this->snapshotProvider_->GetSnapshot()};
        state->SetSnapshot(*snapshot);
        return this->ProvideSnapshotRequest(state);
    }
    if(size)
    {
        //append log entires to request
        request.Entries().Reserve(size);
        while(nextIndex <= lastIndex)
        {
            sharpen::Optional<sharpen::ByteBuffer> log{this->logs_->Lookup(nextIndex)};
            if(!log.Exist())
            {
                std::unique_ptr<sharpen::IRaftSnapshot> snapshot{this->snapshotProvider_->GetSnapshot()};
                state->SetSnapshot(*snapshot);
                return this->ProvideSnapshotRequest(state);
            }
            request.Entries().Push(std::move(log.Get()));
            nextIndex += 1;
        }
        //forward state
        state->Forward(size);
    }
    return this->builder_->BuildHeartbeatRequest(request);
}

sharpen::Mail sharpen::RaftHeartbeatMailProvider::ProvideSynchronizedMail() const
{
    assert(!this->states_.empty());
    std::uint64_t actorId{this->states_.begin()->first};
    return this->Provide(actorId);
}