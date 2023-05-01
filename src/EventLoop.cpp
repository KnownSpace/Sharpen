#include <sharpen/EventLoop.hpp>

#include <cassert>
#include <thread>

thread_local sharpen::EventLoop *sharpen::EventLoop::localLoop_{nullptr};

thread_local sharpen::FiberPtr sharpen::EventLoop::localFiber_{nullptr};

sharpen::EventLoop::EventLoop(SelectorPtr selector)
    : selector_(selector)
    , tasks_()
    , pendingTasks_()
    , exectingTask_(false)
    , lock_()
    , running_(false)
    , works_(0)
    , loopGroup_(nullptr) {
    assert(selector != nullptr);
    this->pendingTasks_.reserve(reservedTaskSize_);
    this->tasks_.reserve(reservedTaskSize_);
}

sharpen::EventLoop::~EventLoop() noexcept {
    this->Stop();
}

void sharpen::EventLoop::Bind(WeakChannelPtr channel) {
    this->selector_->Resister(channel);
}

void sharpen::EventLoop::RunInLoop(Task task) {
    if (this->GetLocalLoop() == this) {
        this->works_ += 1;
        try {
            task();
        } catch (const std::bad_alloc &fault) {
            (void)fault;
            std::terminate();
        } catch (std::system_error &error) {
            if (sharpen::IsFatalError(error.code().value())) {
                std::terminate();
            }
        } catch (const std::exception &ignore) {
            assert(ignore.what() == nullptr && "an exception occured in event loop");
            (void)ignore;
        }
        this->works_ -= 1;
        return;
    }
    this->RunInLoopSoon(std::move(task));
}

void sharpen::EventLoop::RunInLoopSoon(Task task) {
    this->works_ += 1;
    bool execting(true);
    {
        std::unique_lock<Lock> lock(this->lock_);
        this->pendingTasks_.push_back(std::move(task));
        std::swap(execting, this->exectingTask_);
    }
    if (!execting) {
        this->selector_->Notify();
    }
}

void sharpen::EventLoop::ExecuteTask() {
    {
        std::unique_lock<Lock> lock(this->lock_);
        this->exectingTask_ = false;
        if (!this->pendingTasks_.empty()) {
            this->tasks_.swap(this->pendingTasks_);
        }
    }
    for (auto begin = this->tasks_.begin(), end = this->tasks_.end(); begin != end; ++begin) {
        try {
            if (*begin) {
                (*begin)();
            }
        } catch (const std::bad_alloc &fault) {
            (void)fault;
            std::terminate();
        } catch (std::system_error &error) {
            if (sharpen::IsFatalError(error.code().value())) {
                std::terminate();
            }
        } catch (const std::exception &ignore) {
            assert(ignore.what() == nullptr && "an exception occured in event loop");
            (void)ignore;
        }
    }
    this->works_ -= this->tasks_.size();
    this->tasks_.clear();
}

void sharpen::EventLoop::Run() {
    if (sharpen::EventLoop::IsInLoop()) {
        throw std::logic_error("now is in event loop");
    }
    sharpen::EventLoop::localLoop_ = this;
    sharpen::EventLoop::localFiber_ = sharpen::Fiber::GetCurrentFiber();
    EventVector events;
    events.reserve(reservedEventBufSize_);
    this->running_ = true;
    while (this->running_) {
        // select events
        this->selector_->Select(events);
        this->works_ += events.size();
        for (auto begin = events.begin(), end = events.end(); begin != end; ++begin) {
            sharpen::ChannelPtr channel = (*begin)->GetChannel();
            if (channel) {
                channel->OnEvent(*begin);
            }
        }
        this->works_ -= events.size();
        events.clear();
        // execute tasks
        this->ExecuteTask();
    }
    sharpen::EventLoop::localLoop_ = nullptr;
    sharpen::EventLoop::localFiber_.reset();
}

sharpen::FiberPtr sharpen::EventLoop::GetLocalFiber() noexcept {
    return sharpen::EventLoop::localFiber_;
}

void sharpen::EventLoop::Stop() noexcept {
    if (this->running_) {
        this->running_ = false;
        this->selector_->Notify();
    }
}

sharpen::EventLoop *sharpen::EventLoop::GetLocalLoop() noexcept {
    return sharpen::EventLoop::localLoop_;
}

bool sharpen::EventLoop::IsInLoop() noexcept {
    return sharpen::EventLoop::GetLocalLoop() != nullptr;
}

bool sharpen::EventLoop::IsWaiting() const noexcept {
    return !this->works_;
}

std::size_t sharpen::EventLoop::GetWorkCount() const noexcept {
    return this->works_;
}

sharpen::IEventLoopGroup *sharpen::EventLoop::GetCurrentLoopGroup() noexcept {
    if (localLoop_) {
        return localLoop_->GetLoopGroup();
    }
    return nullptr;
}

sharpen::IEventLoopGroup *sharpen::GetLocalLoopGroupPtr() noexcept {
    return sharpen::EventLoop::GetCurrentLoopGroup();
}