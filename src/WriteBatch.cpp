#include <sharpen/WriteBatch.hpp>

sharpen::WriteBatch &sharpen::WriteBatch::operator=(sharpen::WriteBatch &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->actions_ = std::move(other.actions_);
    }
    return *this;
}

void sharpen::WriteBatch::Put(const sharpen::ByteBuffer &key,sharpen::ByteBuffer &&value)
{
    Action action{};
    action.key_ = &key;
    action.value_ = &value;
    action.type_ = sharpen::WriteBatch::ActionType::Put;
    this->actions_.push_back(action);
}

void sharpen::WriteBatch::Delete(const sharpen::ByteBuffer &key)
{
    Action action{};
    action.key_ = &key;
    action.value_ = nullptr;
    action.type_ = sharpen::WriteBatch::ActionType::Delete;
    this->actions_.push_back(action);
}