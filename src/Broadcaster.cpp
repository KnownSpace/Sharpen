#include <sharpen/Broadcaster.hpp>

#include <mutex>

void sharpen::Broadcaster::Cancel() noexcept
{
    for(auto begin = this->actors_.begin(),end = this->actors_.end(); begin != end; ++begin)
    {
        std::unique_ptr<sharpen::IRemoteActor> &actor{begin->second};
        if(actor->GetStatus() == sharpen::RemoteActorStatus::InProgress)
        {
            actor->Cancel();
        }
    }
}

void sharpen::Broadcaster::Broadcast(sharpen::Mail mail)
{
    {
        assert(this->lock_);
        std::unique_lock<Lock> lock{*this->lock_};
        assert(this->sharedMail_);
        this->Cancel();
        *this->sharedMail_ = std::move(mail);
        for(auto begin = this->actors_.begin(),end = this->actors_.end(); begin != end; ++begin)
        {
            std::unique_ptr<sharpen::IRemoteActor> &actor{begin->second};
            actor->PostShared(*this->sharedMail_);
        }
    }
}

void sharpen::Broadcaster::Broadcast(const sharpen::IMailProvider &provider)
{
    {
        assert(this->lock_);
        std::unique_lock<Lock> lock{*this->lock_};
        this->Cancel();
        for(auto begin = this->actors_.begin(),end = this->actors_.end(); begin != end; ++begin)
        {
            std::unique_ptr<sharpen::IRemoteActor> &actor{begin->second};
            sharpen::Mail mail{provider.Provide(actor->GetId())};
            actor->Post(std::move(mail));
        }
    }
}

bool sharpen::Broadcaster::Completed() const noexcept
{
    for(auto begin = this->actors_.begin(),end = this->actors_.end(); begin != end; ++begin)
    {
        const std::unique_ptr<sharpen::IRemoteActor> &actor{begin->second};
        if(actor->GetStatus() == sharpen::RemoteActorStatus::InProgress)
        {
            return false;
        }
    }
    return true;
}

const sharpen::IRemoteActor &sharpen::Broadcaster::GetActor(std::uint64_t actorId) const
{
    auto actor{this->FindActor(actorId)};
    if(!actor)
    {
        throw std::invalid_argument{"unknown actor id"};
    }
    return *actor;
}

bool sharpen::Broadcaster::ExistActor(std::uint64_t actorId) const noexcept
{
    return this->FindActor(actorId);
}

const sharpen::IRemoteActor *sharpen::Broadcaster::FindActor(std::uint64_t actorId) const noexcept
{
    auto ite = this->actors_.find(actorId);
    if(ite != this->actors_.end())
    {
        return ite->second.get();
    }
    return nullptr;
}