#include <sharpen/Quorum.hpp>

sharpen::Quorum &sharpen::Quorum::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->builders_ = std::move(other.builders_);
    }
    return *this;
}

std::unique_ptr<sharpen::Broadcaster> sharpen::Quorum::NviCreateBroadcaster(std::size_t pipeline) const
{
    assert(pipeline > 0);
    std::vector<std::unique_ptr<sharpen::IRemoteActor>> actors;
    actors.reserve(this->builders_.size());
    for(auto begin = this->builders_.begin(),end = this->builders_.end(); begin != end; ++begin)
    {
        std::unique_ptr<sharpen::IRemoteActor> actor{begin->second->Build()};
        if(pipeline > 1 && !actor->SupportPipeline())
        {
            pipeline = 1;
        }
        actors.emplace_back(std::move(actor));
    }
    std::unique_ptr<sharpen::Broadcaster> broadcaster{new (std::nothrow) sharpen::Broadcaster{std::make_move_iterator(actors.begin()),std::make_move_iterator(actors.end()),pipeline}};
    if(!broadcaster)
    {
        throw std::bad_alloc{};
    }
    return broadcaster;
}

sharpen::IRemoteActorBuilder *sharpen::Quorum::NviLookup(std::uint64_t actorId) noexcept
{
    sharpen::IRemoteActorBuilder *builder{nullptr};
    auto ite = this->builders_.find(actorId);
    if(ite != this->builders_.end())
    {
        assert(ite->second != nullptr);
        builder = ite->second.get();
    }
    return builder;
}

const sharpen::IRemoteActorBuilder *sharpen::Quorum::NviLookup(std::uint64_t actorId) const noexcept
{
    sharpen::IRemoteActorBuilder *builder{nullptr};
    auto ite = this->builders_.find(actorId);
    if(ite != this->builders_.end())
    {
        assert(ite->second != nullptr);
        builder = ite->second.get();
    }
    return builder;
}

void sharpen::Quorum::NviRegister(std::uint64_t actorId,std::unique_ptr<sharpen::IRemoteActorBuilder> builder)
{
    auto ite = this->builders_.find(actorId);
    if(ite != this->builders_.end())
    {
        ite->second = std::move(builder);
    }
    else
    {
        this->builders_.emplace(actorId,std::move(builder));
    }
}

void sharpen::Quorum::Remove(std::uint64_t actorId) noexcept
{
    auto ite = this->builders_.find(actorId);
    if(ite != this->builders_.end())
    {
        this->builders_.erase(ite);
    }
}

std::set<std::uint64_t> sharpen::Quorum::GenerateActorsSet() const
{
    std::set<std::uint64_t> actors;
    for(auto begin = this->builders_.begin(),end = this->builders_.end(); begin != end; ++begin)
    {
        actors.emplace(begin->first);   
    }
    return actors;
}