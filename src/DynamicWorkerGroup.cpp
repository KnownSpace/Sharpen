#include <sharpen/DynamicWorkerGroup.hpp>

#include <sharpen/YieldOps.hpp>
#include <cassert>
#include <cstdlib>
#include <new>

void sharpen::DynamicWorkerGroup::Entry(sharpen::AwaitableFuture<void> *future) noexcept {
    std::function<void()> task;
    while (this->token_) {
        task = this->queue_.Pop();
        if (task) {
            sharpen::NonexceptInvoke(task);
        }
        this->taskCount_ -= 1;
    }
    future->Complete();
}

bool sharpen::DynamicWorkerGroup::BusyProbe() const noexcept {
    if (this->token_) {
        for (std::size_t i = 0; i != this->probeCount_; ++i) {
            if (this->taskCount_ <= this->busyMark_) {
                return false;
            }
            if (this->probeCount_ != i + 1) {
                sharpen::YieldCycle();
            }
        }
        return this->token_;
    }
    return false;
}

void sharpen::DynamicWorkerGroup::CreateWorker() {
    std::unique_ptr<sharpen::AwaitableFuture<void>> future{new (std::nothrow)
                                                               sharpen::AwaitableFuture<void>{}};
    if (!future) {
        throw std::bad_alloc{};
    }
    sharpen::AwaitableFuture<void> *worker{nullptr};
    {
        std::unique_lock<sharpen::SpinLock> lock{this->lock_};
        if (this->maxWorkerCount_ && this->workers_.size() + 1 > this->maxWorkerCount_) {
            return;
        }
        if (this->token_) {
            worker = future.get();
            this->workers_.emplace_back(std::move(future));
        }
    }
    if (worker) {
        this->scheduler_->Launch(&Self::Entry, this, worker);
    }
}

void sharpen::DynamicWorkerGroup::NviSubmit(std::function<void()> task) {
    assert(this->token_);
    this->taskCount_ += 1;
    this->queue_.Emplace(std::move(task));
    if (this->BusyProbe()) {
        this->CreateWorker();
    }
}

void sharpen::DynamicWorkerGroup::Stop() noexcept {
    if (this->token_.exchange(false)) {
        std::size_t size{0};
        {
            std::unique_lock<sharpen::SpinLock> lock{this->lock_};
            size = this->workers_.size();
        }
        for (std::size_t i = 0; i != size; ++i) {
            this->taskCount_ += 1;
            this->queue_.Emplace(std::function<void()>{});
        }
    }
}

void sharpen::DynamicWorkerGroup::Join() noexcept {
    std::size_t size{0};
    {
        std::unique_lock<sharpen::SpinLock> lock{this->lock_};
        size = this->workers_.size();
    }
    for (std::size_t i = 0; i != size; ++i) {
        sharpen::AwaitableFuture<void> *future{nullptr};
        {
            std::unique_lock<sharpen::SpinLock> lock{this->lock_};
            future = this->workers_[i].get();
        }
        future->WaitAsync();
    }
}

std::size_t sharpen::DynamicWorkerGroup::GetWorkerCount() const noexcept {
    {
        std::unique_lock<sharpen::SpinLock> lock{this->lock_};
        return this->workers_.size();
    }
}

sharpen::DynamicWorkerGroup::DynamicWorkerGroup(sharpen::IFiberScheduler &scheduler,
                                                std::size_t workerCount,
                                                std::size_t busyMark,
                                                std::size_t probeCount,
                                                std::size_t maxWorkerCount)
    : scheduler_(&scheduler)
    , token_(true)
    , busyMark_(busyMark)
    , lock_()
    , taskCount_(0)
    , workers_()
    , queue_()
    , probeCount_(probeCount)
    , maxWorkerCount_(maxWorkerCount) {
    assert(this->probeCount_);
    assert(this->busyMark_);
    assert(workerCount);
    for (std::size_t i = 0; i != workerCount; ++i) {
        std::unique_ptr<sharpen::AwaitableFuture<void>> future{
            new (std::nothrow) sharpen::AwaitableFuture<void>{}};
        if (!future) {
            throw std::bad_alloc{};
        }
        sharpen::AwaitableFuture<void> *worker{future.get()};
        this->workers_.emplace_back(std::move(future));
        this->scheduler_->Launch(&Self::Entry, this, worker);
    }
}

sharpen::DynamicWorkerGroup::DynamicWorkerGroup(sharpen::IFiberScheduler &scheduler,
                                                std::size_t workerCount,
                                                std::size_t busyMark,
                                                std::size_t probeCount)
    : DynamicWorkerGroup(scheduler, workerCount, busyMark, probeCount, defaultMaxWorkerCount) {
}

sharpen::DynamicWorkerGroup::DynamicWorkerGroup(sharpen::IFiberScheduler &scheduler,
                                                std::size_t workerCount,
                                                std::size_t busyMark)
    : DynamicWorkerGroup(scheduler, workerCount, busyMark, defaultProbeCount) {
}

sharpen::DynamicWorkerGroup::DynamicWorkerGroup(sharpen::IFiberScheduler &scheduler,
                                                std::size_t workerCount)
    : DynamicWorkerGroup(scheduler, workerCount, defaultBusyMark) {
}

sharpen::DynamicWorkerGroup::DynamicWorkerGroup(sharpen::IFiberScheduler &scheduler)
    : DynamicWorkerGroup(scheduler, scheduler.GetParallelCount()) {
}

sharpen::DynamicWorkerGroup::DynamicWorkerGroup()
    : DynamicWorkerGroup(sharpen::GetLocalScheduler()) {
}

sharpen::DynamicWorkerGroup::~DynamicWorkerGroup() noexcept {
    this->Stop();
    this->Join();
}