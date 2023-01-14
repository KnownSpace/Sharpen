#include <sharpen/RaftOption.hpp>

sharpen::RaftOption::RaftOption() noexcept
    :isLearner_(false)
    ,enablePrevote_(false)
{}

sharpen::RaftOption::RaftOption(Self &&other) noexcept
    :isLearner_(other.isLearner_)
    ,enablePrevote_(other.enablePrevote_)
{
    other.isLearner_ = false;
    other.enablePrevote_ = false;
}

sharpen::RaftOption &sharpen::RaftOption::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->isLearner_ = other.isLearner_;
        this->enablePrevote_ = other.enablePrevote_;
        other.isLearner_ = false;
        other.enablePrevote_ = false;
    }
    return *this;
}