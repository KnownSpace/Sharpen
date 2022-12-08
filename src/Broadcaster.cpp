#include <sharpen/Broadcaster.hpp>

#include <mutex>

void sharpen::Broadcaster::Cancel() noexcept
{
    for(auto begin = this->actors_.begin(),end = this->actors_.end(); begin != end; ++begin)
    {
        std::unique_ptr<sharpen::IRemoteActor> &actor{*begin};
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
            std::unique_ptr<sharpen::IRemoteActor> &actor{*begin};
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
            std::unique_ptr<sharpen::IRemoteActor> &actor{*begin};
            sharpen::Mail mail{provider.Provide(actor->GetId())};
            actor->Post(std::move(mail));
        }
    }
}

bool sharpen::Broadcaster::Completed() const noexcept
{
    for(auto begin = this->actors_.begin(),end = this->actors_.end(); begin != end; ++begin)
    {
        const std::unique_ptr<sharpen::IRemoteActor> &actor{*begin};
        if(actor->GetStatus() == sharpen::RemoteActorStatus::InProgress)
        {
            return false;
        }
    }
    return true;
}