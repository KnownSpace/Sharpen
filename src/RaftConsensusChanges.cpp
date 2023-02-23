#include <sharpen/RaftConsensusChanges.hpp>

void sharpen::RaftConsensusChanges::NviInsertMachine(std::uint64_t actorId,sharpen::ByteBuffer log)
{
    assert(this->logBatch_);
    assert(this->Insertable());
    this->insertSet_.Insert(actorId,log);
}

void sharpen::RaftConsensusChanges::NviRemoveMachine(std::uint64_t actorId,sharpen::ByteBuffer log)
{
    assert(this->logBatch_);
    assert(this->Removeable());
    this->removeSet_.Insert(actorId,log);
}

void sharpen::RaftConsensusChanges::NviMoveToBindedBatch()
{
    assert(this->logBatch_);
    std::size_t size{this->insertSet_.GetSize() + this->removeSet_.GetSize()};
    this->logBatch_->Reverse(size);
    for(auto begin = this->insertSet_.Begin(),end = this->insertSet_.End(); begin != end; ++begin)
    {
        this->logBatch_->Append(std::move(begin->second));
    }
    for(auto begin = this->removeSet_.Begin(),end = this->removeSet_.End(); begin != end; ++begin)
    {
        this->logBatch_->Append(std::move(begin->second));
    }
    this->insertSet_.Clear();
    this->removeSet_.Clear();
}

bool sharpen::RaftConsensusChanges::Insertable() const noexcept
{
    assert(this->logBatch_);
    //only apply one machine at once
    return this->mode_ != sharpen::RaftConsensusChanges::Mode::Apply || this->insertSet_.GetSize() < 1;
}

bool sharpen::RaftConsensusChanges::Removeable() const noexcept
{
    assert(this->logBatch_);
    return true;
}

const sharpen::MachineSet &sharpen::RaftConsensusChanges::GetInsertSet() const noexcept
{
    return this->insertSet_;
}

const sharpen::MachineSet &sharpen::RaftConsensusChanges::GetRemoveSet() const noexcept
{
    return this->removeSet_;
}

sharpen::RaftConsensusChanges::RaftConsensusChanges(Mode mode,sharpen::ILogBatch &batch)
    :mode_(mode)
    ,insertSet_()
    ,removeSet_()
    ,logBatch_(&batch)
{}

sharpen::RaftConsensusChanges::RaftConsensusChanges(Self &&other) noexcept
    :mode_(other.mode_)
    ,insertSet_(std::move(other.insertSet_))
    ,removeSet_(std::move(other.removeSet_))
    ,logBatch_(other.logBatch_)
{
    other.mode_ = sharpen::RaftConsensusChanges::Mode::Prepare;
    other.logBatch_ = nullptr;
}

sharpen::RaftConsensusChanges &sharpen::RaftConsensusChanges::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->mode_ = other.mode_;
        this->insertSet_ = std::move(other.insertSet_);
        this->removeSet_ = std::move(other.removeSet_);
        this->logBatch_ = other.logBatch_;
        other.mode_ = sharpen::RaftConsensusChanges::Mode::Prepare;
        other.logBatch_ = nullptr;
    }
    return *this;
}

