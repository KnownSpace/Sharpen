#include <sharpen/TcpActor.hpp>

#include <sharpen/RemotePosterOpenError.hpp>
#include <sharpen/SingleWorkerGroup.hpp>
#include <sharpen/SystemError.hpp>
#include <sharpen/YieldOps.hpp>
#include <new>

void sharpen::TcpActor::DoReceive(sharpen::Mail response) noexcept {
    if (!response.Empty()) {
        this->receiver_->Receive(std::move(response), this->GetId());
    }
    this->ackCount_.fetch_add(1, std::memory_order::memory_order_acq_rel);
}

void sharpen::TcpActor::DoPostShared(const sharpen::Mail *mail) noexcept {
    assert(mail);
    this->postCount_.fetch_add(1, std::memory_order::memory_order_acq_rel);
    if (!this->poster_->Available()) {
        try {
            std::unique_ptr<sharpen::IMailParser> parser{this->parserFactory_->Produce()};
            this->poster_->Open(std::move(parser));
        } catch (const sharpen::RemotePosterOpenError &ignore) {
            (void)ignore;
            this->ackCount_.fetch_add(1, std::memory_order::memory_order_acq_rel);
            return;
        } catch (const std::system_error &error) {
            sharpen::ErrorCode errorCode{sharpen::GetErrorCode(error)};
            if (sharpen::IsFatalError(errorCode)) {
                std::terminate();
            }
            assert(!error.what() && "fail to post mail");
            (void)error;
            this->ackCount_.fetch_add(1, std::memory_order::memory_order_acq_rel);
            return;
        } catch (const std::exception &ignore) {
            assert(!ignore.what() && "fail to post mail");
            (void)ignore;
            this->ackCount_.fetch_add(1, std::memory_order::memory_order_acq_rel);
            return;
        }
    }
    if (this->pipelineCb_) {
        return this->poster_->Post(*mail, this->pipelineCb_);
    }
    sharpen::Mail response{this->poster_->Post(*mail)};
    this->DoReceive(std::move(response));
}

void sharpen::TcpActor::DoPost(sharpen::Mail mail) noexcept {
    this->DoPostShared(&mail);
}

sharpen::RemoteActorStatus sharpen::TcpActor::GetStatus() const noexcept {
    std::size_t ackCount{this->ackCount_.load(std::memory_order::memory_order_acquire)};
    std::size_t postCount{this->postCount_.load(std::memory_order::memory_order_acquire)};
    if (postCount != ackCount) {
        return sharpen::RemoteActorStatus::InProgress;
    }
    if (this->poster_->Available()) {
        return sharpen::RemoteActorStatus::Opened;
    }
    return sharpen::RemoteActorStatus::Closed;
}

std::size_t sharpen::TcpActor::GetPipelineCount() const noexcept {
    std::size_t ackCount{this->ackCount_.load(std::memory_order::memory_order_acquire)};
    std::size_t postCount{this->postCount_.load(std::memory_order::memory_order_acquire)};
    assert(postCount >= ackCount);
    return postCount - ackCount;
}

void sharpen::TcpActor::Drain() noexcept {
    std::size_t ackCount{this->ackCount_.load(std::memory_order::memory_order_acquire)};
    std::size_t postCount{this->postCount_.load(std::memory_order::memory_order_acquire)};
    assert(postCount >= ackCount);
    if (ackCount != postCount) {
        if (this->poster_->Available()) {
            this->poster_->Close();
            // wait for pipeline empty
            ackCount = this->ackCount_.load(std::memory_order::memory_order_acquire);
            std::size_t newPostCount{
                this->postCount_.load(std::memory_order::memory_order_acquire)};
            postCount = newPostCount;
            while (ackCount != newPostCount) {
                sharpen::YieldCycleForBusyLoop();
                ackCount = this->ackCount_.load(std::memory_order::memory_order_acquire);
                newPostCount = this->postCount_.load(std::memory_order::memory_order_acquire);
                if (postCount != newPostCount) {
                    this->poster_->Close();
                }
            }
        }
    }
}

bool sharpen::TcpActor::SupportPipeline() const noexcept {
    return this->pipelineCb_ != nullptr;
}

void sharpen::TcpActor::Cancel() noexcept {
    std::size_t ackCount{this->ackCount_.load(std::memory_order::memory_order_acquire)};
    std::size_t postCount{this->postCount_.load(std::memory_order::memory_order_acquire)};
    // if pipeline is not empty
    if (postCount != ackCount) {
        // if poster is avaliable
        if (this->poster_->Available()) {
            // close poster
            this->poster_->Close();
            // wait for pipeline empty
            ackCount = this->ackCount_.load(std::memory_order::memory_order_acquire);
            postCount = this->postCount_.load(std::memory_order::memory_order_acquire);
            while (ackCount < postCount) {
                sharpen::YieldCycleForBusyLoop();
                ackCount = this->ackCount_.load(std::memory_order::memory_order_acquire);
            }
        }
    }
}

void sharpen::TcpActor::Close() noexcept {
    if (this->poster_->Available()) {
        std::size_t ackCount{this->ackCount_.load(std::memory_order::memory_order_acquire)};
        std::size_t postCount{this->postCount_.load(std::memory_order::memory_order_acquire)};
        // close poster
        this->poster_->Close();
        // if pipeline is not empty
        if (postCount != ackCount) {
            // wait for pipeline empty
            ackCount = this->ackCount_.load(std::memory_order::memory_order_acquire);
            postCount = this->postCount_.load(std::memory_order::memory_order_acquire);
            while (ackCount < postCount) {
                sharpen::YieldCycleForBusyLoop();
                ackCount = this->ackCount_.load(std::memory_order::memory_order_acquire);
            }
        }
    }
}

void sharpen::TcpActor::NviPost(sharpen::Mail mail) {
    this->postWorker_->Submit(&Self::DoPost, this, std::move(mail));
}

void sharpen::TcpActor::NviPostShared(const sharpen::Mail &mail) {
    this->postWorker_->Submit(&Self::DoPostShared, this, &mail);
}

sharpen::TcpActor::TcpActor(sharpen::IFiberScheduler &scheduler,
                            sharpen::IMailReceiver &receiver,
                            std::shared_ptr<sharpen::IMailParserFactory> factory,
                            std::unique_ptr<sharpen::IRemotePoster> poster)
    : Self{scheduler, receiver, std::move(factory), std::move(poster), false} {
}

sharpen::TcpActor::TcpActor(sharpen::IFiberScheduler &scheduler,
                            sharpen::IMailReceiver &receiver,
                            std::shared_ptr<sharpen::IMailParserFactory> factory,
                            std::unique_ptr<sharpen::IRemotePoster> poster,
                            bool enablePipeline)
    : receiver_(&receiver)
    , postCount_(0)
    , ackCount_(0)
    , parserFactory_(std::move(factory))
    , pipelineCb_()
    , poster_(std::move(poster))
    , postWorker_(nullptr) {
    assert(this->parserFactory_);
    assert(this->poster_);
    sharpen::IWorkerGroup *worker{new (std::nothrow) sharpen::SingleWorkerGroup{scheduler}};
    if (!worker) {
        throw std::bad_alloc{};
    }
    this->postWorker_.reset(worker);
    if (enablePipeline && this->poster_->SupportPipeline()) {
        this->pipelineCb_ = std::bind(&Self::DoReceive, this, std::placeholders::_1);
    }
}

sharpen::TcpActor::~TcpActor() noexcept {
    this->poster_->Close();
}