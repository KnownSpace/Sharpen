#include <sharpen/RaftHeartbeatMailProvider.hpp>

sharpen::RaftHeartbeatMailProvider::RaftHeartbeatMailProvider(std::uint64_t id,const sharpen::IRaftMailBuilder &builder,const sharpen::ILogStorage &log)
    :Self{id,builder,log,defaultBatchSize_,defaultPipelineLength_}
{}

sharpen::RaftHeartbeatMailProvider::RaftHeartbeatMailProvider(std::uint64_t id,const sharpen::IRaftMailBuilder &builder,const sharpen::ILogStorage &log,std::uint16_t batchSize,std::uint16_t pipelineLength)
    :id_(id)
    ,builder_(&builder)
    ,log_(&log)
    ,batchSize_(batchSize)
    ,pipelineLength_(pipelineLength)
    ,nextIndexs_()
    ,matchIndexs_()
    ,term_(0)
    ,commitIndex_(0)
{
    assert(this->batchSize_ >= 1);
    assert(this->pipelineLength_ >= 1);
}

sharpen::RaftHeartbeatMailProvider::RaftHeartbeatMailProvider(Self &&other) noexcept
    :id_(other.id_)
    ,builder_(other.builder_)
    ,log_(other.log_)
    ,batchSize_(other.batchSize_)
    ,pipelineLength_(other.pipelineLength_)
    ,nextIndexs_(std::move(other.nextIndexs_))
    ,matchIndexs_(std::move(other.matchIndexs_))
    ,term_(other.term_)
    ,commitIndex_(other.commitIndex_)
{
    other.id_ = 0;
    other.builder_ = nullptr;
    other.log_ = nullptr;
    other.term_ = 0;
    other.commitIndex_ = 0;
    other.batchSize_ = 0;
    other.pipelineLength_ = 0;
}

sharpen::RaftHeartbeatMailProvider &sharpen::RaftHeartbeatMailProvider::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->id_ = other.id_;
        this->builder_ = other.builder_;
        this->log_ = other.log_;
        this->batchSize_ = other.batchSize_;
        this->pipelineLength_ = other.pipelineLength_;
        this->nextIndexs_ = std::move(other.nextIndexs_);
        this->matchIndexs_ = std::move(other.matchIndexs_);
        this->term_ = other.term_;
        this->commitIndex_ = other.commitIndex_;
        other.id_ = 0;
        other.builder_ = nullptr;
        other.log_ = nullptr;
        other.term_ = 0;
        other.commitIndex_ = 0;
        other.batchSize_ = 0;
        other.pipelineLength_ = 0;
    }
    return *this;
}

sharpen::Optional<std::uint64_t> sharpen::RaftHeartbeatMailProvider::LookupIndex(std::uint64_t actorId) const noexcept
{
    auto ite = this->nextIndexs_.find(actorId);
    if(ite != this->nextIndexs_.end())
    {
        return ite->second;
    }
    return sharpen::EmptyOpt;
}

void sharpen::RaftHeartbeatMailProvider::SetIndex(std::uint64_t actorId,std::uint64_t index)
{
    auto ite = this->nextIndexs_.find(actorId);
    if(ite != this->nextIndexs_.end())
    {
        ite->second = index;
    }
    else
    {
        this->nextIndexs_.emplace(actorId,index);
    }
}

void sharpen::RaftHeartbeatMailProvider::RemoveIndex(std::uint64_t actorId) noexcept
{
    auto ite = this->nextIndexs_.find(actorId);
    if(ite != this->nextIndexs_.end())
    {
        this->nextIndexs_.erase(ite);
    }
}

std::size_t sharpen::RaftHeartbeatMailProvider::GetSize() const noexcept
{
    return this->nextIndexs_.size();
}

bool sharpen::RaftHeartbeatMailProvider::Empty() const noexcept
{
    return this->nextIndexs_.empty();
}

sharpen::Optional<std::uint64_t> sharpen::RaftHeartbeatMailProvider::GetSynchronizedIndex() const noexcept
{
    if(!this->nextIndexs_.empty())
    {
        auto begin = this->nextIndexs_.begin();
        auto end = this->nextIndexs_.end();
        std::uint64_t index{begin->second};
        ++begin;
        while (begin != end)
        {
            if(begin->second != index)
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
    if(!this->matchIndexs_.empty())
    {
        std::size_t majority{this->matchIndexs_.size()};
        majority += 1;
        majority /= 2;
        for(auto begin = this->matchIndexs_.begin(),end = this->matchIndexs_.end(); begin != end; ++begin)
        {
            if(begin->second > index)
            {
                auto ite = begin;
                ++ite;
                std::size_t count{1};
                while(ite != end)
                {
                    if(ite->second >= begin->second)
                    {
                        count += 1;
                    }
                    ++ite;
                }
                if(count >= majority)
                {
                    index = begin->second;
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

sharpen::Mail sharpen::RaftHeartbeatMailProvider::Provide(std::uint64_t actorId) const
{
    //TODO pipeline
    assert(this->builder_);
    assert(this->log_);
    assert(this->batchSize_ >= 1);
    assert(this->pipelineLength_ >= 1);
    sharpen::Optional<std::uint64_t> index{this->LookupIndex(actorId)};
    if(!index.Exist())
    {
        return sharpen::Mail{};
    }
    std::uint64_t realIndex{index.Get()};
    std::uint64_t lastIndex{this->log_->GetLastIndex()};
    std::uint64_t preIndex{realIndex};
    if(preIndex)
    {
        preIndex -= 1;
    }
    assert(realIndex <= lastIndex);
    std::uint64_t size{lastIndex - realIndex};
    size = (std::min)(this->batchSize_,size);
    lastIndex = realIndex + size;
    sharpen::RaftHeartbeatRequest request;
    std::uint64_t commitIndex{(std::min)(this->GetCommitIndex(),lastIndex)};
    request.SetCommitIndex(commitIndex);
    request.SetLeaderId(this->id_);
    request.SetTerm(this->term_);
    request.SetPreLogIndex(preIndex);
    sharpen::Optional<std::uint64_t> term{this->log_->LookupTerm(preIndex)};
    if(!term.Exist())
    {
        //FIXME:Send state machine snapshot
    }
    if(size)
    {
        request.Entries().Reserve(size);
        while(realIndex <= lastIndex)
        {
            sharpen::Optional<sharpen::ByteBuffer> log{this->log_->Lookup(realIndex)};
            if(!log.Exist())
            {
                //FIXME:Send state machine snapshot
            }
            request.Entries().Push(std::move(log.Get()));
            realIndex += 1;
        }
    }
    return this->builder_->BuildHeartbeatRequest(request);
}

sharpen::Mail sharpen::RaftHeartbeatMailProvider::ProvideSynchronizedMail() const
{
    assert(!this->nextIndexs_.empty());
    std::uint64_t index{this->nextIndexs_.begin()->second};
    return this->Provide(index);
}