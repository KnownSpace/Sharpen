#include <sharpen/MachineSet.hpp>

void sharpen::MachineSet::Insert(std::uint64_t actorId, sharpen::ByteBuffer data)
{
    auto ite = this->map_.find(actorId);
    if (ite != this->map_.end())
    {
        ite->second = std::move(data);
        return;
    }
    this->map_.emplace(actorId, std::move(data));
}

void sharpen::MachineSet::Remove(std::uint64_t actorId) noexcept
{
    auto ite = this->map_.find(actorId);
    if (ite != this->map_.end())
    {
        this->map_.erase(ite);
    }
}

std::size_t sharpen::MachineSet::GetSize() const noexcept
{
    return this->map_.size();
}

bool sharpen::MachineSet::Empty() const noexcept
{
    return this->map_.empty();
}

void sharpen::MachineSet::Clear() noexcept
{
    this->map_.clear();
}