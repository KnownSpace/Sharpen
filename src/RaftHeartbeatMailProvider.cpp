#include <sharpen/RaftHeartbeatMailProvider.hpp>

sharpen::RaftHeartbeatMailProvider::RaftHeartbeatMailProvider(std::uint64_t id,const sharpen::IRaftMailBuilder &builder,const sharpen::ILogStorage &log)
    :id_(id)
    ,builder_(&builder)
    ,log_(&log)
    ,nextIndexs_()
    ,term_(0)
    ,commitIndex_(0)
{}

sharpen::RaftHeartbeatMailProvider::RaftHeartbeatMailProvider(const Self &other)
    :id_(other.id_)
    ,builder_(other.builder_)
    ,log_(other.log_)
    ,nextIndexs_(other.nextIndexs_)
    ,term_(other.term_)
    ,commitIndex_(other.commitIndex_)
{}

sharpen::RaftHeartbeatMailProvider::RaftHeartbeatMailProvider(Self &&other) noexcept
    :id_(other.id_)
    ,builder_(other.builder_)
    ,log_(other.log_)
    ,nextIndexs_(std::move(other.nextIndexs_))
    ,term_(other.term_)
    ,commitIndex_(other.commitIndex_)
{
    other.id_ = 0;
    other.builder_ = nullptr;
    other.log_ = nullptr;
    other.term_ = 0;
    other.commitIndex_ = 0;
}

sharpen::RaftHeartbeatMailProvider &sharpen::RaftHeartbeatMailProvider::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->id_ = other.id_;
        this->builder_ = other.builder_;
        this->log_ = other.log_;
        this->nextIndexs_ = std::move(other.nextIndexs_);
        this->term_ = other.term_;
        this->commitIndex_ = other.commitIndex_;
        other.id_ = 0;
        other.builder_ = nullptr;
        other.log_ = nullptr;
        other.term_ = 0;
        other.commitIndex_ = 0;
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

sharpen::Mail sharpen::RaftHeartbeatMailProvider::Provide(std::uint64_t actorId) const
{
    assert(this->builder_);
    assert(this->log_);
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
    sharpen::RaftHeartbeatRequest request;
    request.SetCommitIndex(this->commitIndex_);
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
        while(realIndex != lastIndex)
        {
            sharpen::Optional<sharpen::ByteBuffer> log{this->log_->Lookup(realIndex)};
            if(!log.Exist())
            {
                //FIXME:Send state machine snapshot
            }
            request.Entries().Push(std::move(log.Get()));
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