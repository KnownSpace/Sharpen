#include <sharpen/Broadcaster.hpp>

#include <mutex>

sharpen::Broadcaster::Broadcaster(std::unique_ptr<sharpen::IRemoteActor> *actor,
                                  std::size_t size,
                                  std::size_t pipeline)
    : lock_(nullptr)
    , index_(0)
    , pipelineLength_(pipeline)
    , sharedMails_()
    , actors_() {
    this->lock_.reset(new (std::nothrow) Lock{});
    if (!this->lock_) {
        throw std::bad_alloc{};
    }
    this->actors_.rehash(size);
    for (std::size_t i = 0; i != size; ++i) {
        std::uint64_t id{actor[i]->GetId()};
        this->actors_.emplace(id, std::move(actor[i]));
    }
    assert(this->pipelineLength_ > 0);
    this->sharedMails_.resize(this->pipelineLength_);
}

void sharpen::Broadcaster::Cancel() noexcept {
    for (auto begin = this->actors_.begin(), end = this->actors_.end(); begin != end; ++begin) {
        std::unique_ptr<sharpen::IRemoteActor> &actor{begin->second};
        if (actor->GetPipelineCount() == this->pipelineLength_) {
            actor->Cancel();
        }
    }
}

void sharpen::Broadcaster::Drain() noexcept {
    for (auto begin = this->actors_.begin(), end = this->actors_.end(); begin != end; ++begin) {
        std::unique_ptr<sharpen::IRemoteActor> &actor{begin->second};
        actor->Drain();
    }
}

sharpen::Broadcaster::~Broadcaster() noexcept {
    this->Cancel();
}

sharpen::Mail *sharpen::Broadcaster::GetNextSharedMail() noexcept {
    assert(!this->sharedMails_.empty());
    std::size_t index{this->index_ % this->sharedMails_.size()};
    this->index_ += 1;
    return &this->sharedMails_[index];
}

void sharpen::Broadcaster::Broadcast(sharpen::Mail mail) {
    {
        assert(this->pipelineLength_ != 0);
        assert(this->lock_);
        std::unique_lock<Lock> lock{*this->lock_};
        this->Cancel();
        sharpen::Mail *sharedMail{this->GetNextSharedMail()};
        assert(sharedMail);
        *sharedMail = std::move(mail);
        for (auto begin = this->actors_.begin(), end = this->actors_.end(); begin != end; ++begin) {
            std::unique_ptr<sharpen::IRemoteActor> &actor{begin->second};
            actor->PostShared(*sharedMail);
        }
    }
}

void sharpen::Broadcaster::Broadcast(const sharpen::IMailProvider &provider) {
    {
        assert(this->pipelineLength_ != 0);
        assert(this->lock_);
        std::unique_lock<Lock> lock{*this->lock_};
        this->Cancel();
        for (auto begin = this->actors_.begin(), end = this->actors_.end(); begin != end; ++begin) {
            std::unique_ptr<sharpen::IRemoteActor> &actor{begin->second};
            sharpen::Mail mail{provider.Provide(actor->GetId())};
            actor->Post(std::move(mail));
        }
    }
}

bool sharpen::Broadcaster::Completed() const noexcept {
    for (auto begin = this->actors_.begin(), end = this->actors_.end(); begin != end; ++begin) {
        const std::unique_ptr<sharpen::IRemoteActor> &actor{begin->second};
        if (actor->GetPipelineCount()) {
            return false;
        }
    }
    return true;
}

const sharpen::IRemoteActor &sharpen::Broadcaster::GetActor(std::uint64_t actorId) const {
    auto actor{this->FindActor(actorId)};
    if (!actor) {
        throw std::invalid_argument{"unknown actor id"};
    }
    return *actor;
}

bool sharpen::Broadcaster::ExistActor(std::uint64_t actorId) const noexcept {
    return this->FindActor(actorId);
}

const sharpen::IRemoteActor *sharpen::Broadcaster::FindActor(std::uint64_t actorId) const noexcept {
    auto ite = this->actors_.find(actorId);
    if (ite != this->actors_.end()) {
        return ite->second.get();
    }
    return nullptr;
}

std::size_t sharpen::Broadcaster::GetPipelineLength() const noexcept {
    return this->pipelineLength_;
}

bool sharpen::Broadcaster::SupportPipeline() const noexcept {
    return this->pipelineLength_ > 1;
}