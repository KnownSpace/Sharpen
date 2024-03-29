#include <sharpen/SingleWorkerGroup.hpp>

sharpen::SingleWorkerGroup::SingleWorkerGroup()
    : SingleWorkerGroup(sharpen::GetLocalScheduler()) {
}

sharpen::SingleWorkerGroup::SingleWorkerGroup(sharpen::IFiberScheduler &scheduler)
    : token_(true)
    , queue_()
    , worker_() {
    scheduler.Launch(&Self::Entry, this);
}

sharpen::SingleWorkerGroup::~SingleWorkerGroup() noexcept {
    this->Stop();
    this->Join();
}

void sharpen::SingleWorkerGroup::Stop() noexcept {
    if (this->token_.exchange(false)) {
        this->queue_.EmplaceBack(std::function<void()>{});
    }
}

void sharpen::SingleWorkerGroup::Join() noexcept {
    if (this->worker_.IsPending()) {
        this->worker_.WaitAsync();
    }
}

bool sharpen::SingleWorkerGroup::Running() const noexcept {
    return this->token_;
}

void sharpen::SingleWorkerGroup::Entry() noexcept {
    std::function<void()> task;
    while (this->token_) {
        task = this->queue_.Pop();
        if (task) {
            sharpen::NonexceptInvoke(task);
        }
    }
    this->worker_.Complete();
}

void sharpen::SingleWorkerGroup::NviSubmit(std::function<void()> task) {
    assert(this->token_);
    this->queue_.EmplaceBack(std::move(task));
}

void sharpen::SingleWorkerGroup::NviSubmitUrgent(std::function<void()> task) {
    assert(this->token_);
    this->queue_.EmplaceFront(std::move(task));
}