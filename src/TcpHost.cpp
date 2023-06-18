#include <sharpen/TcpHost.hpp>

#include <sharpen/YieldOps.hpp>

sharpen::TcpHost::TcpHost(sharpen::ITcpSteamFactory &factory)
    : Self{sharpen::GetLocalScheduler(), factory} {
}

sharpen::TcpHost::TcpHost(sharpen::IFiberScheduler &scheduler, sharpen::ITcpSteamFactory &factory)
    : scheduler_(&scheduler)
    , loopGroup_(&factory.GetLoopGroup())
    , token_(false)
    , pipeline_(nullptr)
    , acceptor_(nullptr) {
    sharpen::NetStreamChannelPtr channel{factory.Produce()};
    // reuse address
    channel->SetReuseAddress(true);
    channel->Listen(65535);
    this->acceptor_ = std::move(channel);
}

sharpen::TcpHost::~TcpHost() noexcept {
    this->Stop();
}

void sharpen::TcpHost::NviSetPipeline(std::unique_ptr<sharpen::IHostPipeline> pipeline) noexcept {
    this->pipeline_ = std::move(pipeline);
}

void sharpen::TcpHost::ConsumeChannel(sharpen::NetStreamChannelPtr channel,
                                      std::atomic_size_t *counter) noexcept {
    this->pipeline_->Consume(std::move(channel));
    counter->fetch_sub(1, std::memory_order_relaxed);
}

void sharpen::TcpHost::Stop() noexcept {
    this->pipeline_->Stop();
    this->token_ = false;
    this->acceptor_->Close();
}

void sharpen::TcpHost::Run() {
    this->token_ = true;
    assert(this->pipeline_);
    std::atomic_size_t counter{0};
    while (this->token_) {
        sharpen::NetStreamChannelPtr channel{nullptr};
        try {
            channel = this->acceptor_->AcceptAsync();
        } catch (const std::system_error &error) {
            sharpen::ErrorCode code{sharpen::GetErrorCode(error)};
            if (sharpen::IsFatalError(code)) {
                std::terminate();
            }
            if (code != sharpen::ErrorConnectionAborted && code != sharpen::ErrorCancel && code != sharpen::ErrorConnectionReset) {
                throw;
            }
        }
        if (this->token_ && channel) {
            channel->Register(*this->loopGroup_);
            counter.fetch_add(1, std::memory_order_relaxed);
            // launch
            this->scheduler_->Launch(&Self::ConsumeChannel, this, std::move(channel), &counter);
        }
    }
    // FIXME:busy loop
    while (counter.load(std::memory_order_relaxed) != 0) {
        sharpen::YieldCycle();
    }
}