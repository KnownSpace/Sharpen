#include <sharpen/TcpPoster.hpp>

#include <sharpen/SystemError.hpp>
#include <cassert>
#include <new>

#ifndef _NDEBUG
#include <sharpen/DebugTools.hpp>
#endif

void sharpen::TcpPoster::NviClose() noexcept {
    sharpen::NetStreamChannelPtr channel{nullptr};
    {
        assert(this->lock_);
        std::unique_lock<sharpen::SpinLock> lock{*this->lock_};
        std::swap(channel, this->channel_);
    }
    if (channel) {
        channel->Close();
    }
}

void sharpen::TcpPoster::NviOpen(std::unique_ptr<sharpen::IMailParser> parser) {
    this->parser_ = std::move(parser);
    sharpen::NetStreamChannelPtr channel{nullptr};
    {
        assert(this->lock_);
        std::unique_lock<sharpen::SpinLock> lock{*this->lock_};
        if (!this->channel_) {
            assert(this->factory_);
            this->channel_ = this->factory_->Produce(sharpen::TcpStreamOption{});
        }
        channel = this->channel_;
    }
    assert(channel);
    try {
        channel->ConnectAsync(*this->remoteEndpoint_);
    } catch (const std::system_error &error) {
        sharpen::ErrorCode errorCode{sharpen::GetErrorCode(error)};
        switch (errorCode) {
        case sharpen::ErrorIsConnected:
            return;
        case sharpen::ErrorConnectionAborted:
            throw sharpen::RemotePosterOpenError{"connection aborted"};
            break;
        case sharpen::ErrorCancel:
            throw sharpen::RemotePosterOpenError{"poster is closed by operator"};
            break;
        case sharpen::ErrorConnectionRefused:
            throw sharpen::RemotePosterOpenError{"connection refused"};
            break;
        case sharpen::ErrorHostUnreachable:
            throw sharpen::RemotePosterOpenError{"unreachable host"};
            break;
        case sharpen::ErrorNetUnreachable:
            throw sharpen::RemotePosterOpenError{"unreachable network"};
            break;
        case sharpen::ErrorConnectionReset:
            throw sharpen::RemotePosterOpenError{"connection reset"};
            break;
        case sharpen::ErrorNotSocket:
            throw sharpen::RemotePosterOpenError{"poster is closed by operator"};
            break;
        case sharpen::ErrorBrokenPipe:
            throw sharpen::RemotePosterOpenError{"poster is closed by operator"};
            break;
        case sharpen::ErrorBadFileHandle:
            throw sharpen::RemotePosterOpenError{"poster is closed by operator"};
            break;
#ifdef SHARPEN_IS_WIN
        case sharpen::ErrorBadSocketHandle:
            throw sharpen::RemotePosterOpenError{"poster is closed by operator"};
            break;
#endif
        }
        throw;
    } catch (const std::exception &rethrow) {
        (void)rethrow;
        throw;
    }
}

bool sharpen::TcpPoster::Available() const noexcept {
    {
        std::unique_lock<sharpen::SpinLock> lock{*this->lock_};
        return this->channel_ != nullptr;
    }
}

bool sharpen::TcpPoster::SupportPipeline() const noexcept {
    return this->pipelineWorker_ != nullptr;
}

void sharpen::TcpPoster::AbortConn(sharpen::INetStreamChannel *conn) noexcept {
    {
        assert(this->lock_);
        std::unique_lock<sharpen::SpinLock> lock{*this->lock_};
        if (this->channel_.get() == conn) {
            this->channel_.reset();
        }
        conn->Close();
    }
}

sharpen::Mail sharpen::TcpPoster::DoReceive(sharpen::NetStreamChannelPtr channel) noexcept {
    std::size_t size{0};
    sharpen::ByteBuffer buffer{4096};
    sharpen::Mail response;
    while (!this->parser_->Completed()) {
        size = channel->ReadAsync(buffer);
        if (!size) {
            // connection was aborted
            this->AbortConn(channel.get());
            return sharpen::Mail{};
        }
        sharpen::ByteSlice slice{buffer.Data(), size};
        try {
            this->parser_->Parse(slice);
        } catch (const sharpen::MailParseError &error) {
            // connection was aborted
            this->AbortConn(channel.get());
            (void)error;
            return sharpen::Mail{};
        }
    }
    response = this->parser_->PopCompletedMail();
    return response;
}

void sharpen::TcpPoster::Receive(sharpen::NetStreamChannelPtr channel,
                                 std::function<void(sharpen::Mail)> cb) noexcept {
    sharpen::Mail response{this->DoReceive(channel)};
    cb(std::move(response));
}

void sharpen::TcpPoster::NviPost(const sharpen::Mail &mail,
                                 std::function<void(sharpen::Mail)> cb) noexcept {
    sharpen::NetStreamChannelPtr channel{nullptr};
    {
        assert(this->lock_);
        std::unique_lock<sharpen::SpinLock> lock{*this->lock_};
        channel = this->channel_;
    }
    if (!channel) {
        cb(sharpen::Mail{});
        return;
    }
    // post mail
    std::size_t size{0};
#ifdef SHARPEN_IS_WIN
    try {
        size = channel->WriteFixedAsync(mail.Header());
    } catch (const std::system_error &error) {
        (void)error;
#ifndef _NDEBUG
        if (sharpen::GetErrorCode(error) != sharpen::ErrorNotConnected) {
            assert("failed to write mail header to channel" && false);
        }
#endif
    }
#else
    size = channel->WriteFixedAsync(mail.Header());
#endif
    if (!size) {
        cb(sharpen::Mail{});
        return;
    }
    if (!mail.Content().Empty()) {
        size = 0;
#ifdef SHARPEN_IS_WIN
        try {
            size = channel->WriteFixedAsync(mail.Content());
        } catch (const std::system_error &error) {
            (void)error;
#ifndef _NDEBUG
            if (sharpen::GetErrorCode(error) != sharpen::ErrorNotConnected) {
                assert("failed to write mail content to channel" && false);
            }
#endif
        }
#else
        size = channel->WriteFixedAsync(mail.Content());
#endif
        if (!size) {
            cb(sharpen::Mail{});
            return;
        }
    }
    // receive mail
    if (!this->pipelineWorker_) {
        sharpen::Mail response{this->DoReceive(std::move(channel))};
        cb(std::move(response));
        return;
    }
    // pipeline model
    this->pipelineWorker_->Submit(&Self::Receive, this, std::move(channel), std::move(cb));
}

sharpen::Mail sharpen::TcpPoster::NviPost(const sharpen::Mail &mail) noexcept {
    sharpen::NetStreamChannelPtr channel{nullptr};
    {
        assert(this->lock_);
        std::unique_lock<sharpen::SpinLock> lock{*this->lock_};
        channel = this->channel_;
    }
    if (!channel) {
        return sharpen::Mail{};
    }
    // post mail
    std::size_t size{0};
#ifdef SHARPEN_IS_WIN
    try {
        size = channel->WriteFixedAsync(mail.Header());
    } catch (const std::system_error &error) {
        (void)error;
#ifndef _NDEBUG
        if (sharpen::GetErrorCode(error) != sharpen::ErrorNotConnected) {
            assert("failed to write mail header to channel" && false);
        }
#endif
    }
#else
    size = channel->WriteFixedAsync(mail.Header());
#endif
    if (!size) {
        AbortConn(channel.get());
        return sharpen::Mail{};
    }
    if (!mail.Content().Empty()) {
        size = 0;
#ifdef SHARPEN_IS_WIN
        try {
            size = channel->WriteFixedAsync(mail.Content());
        } catch (const std::system_error &error) {
            (void)error;
#ifndef _NDEBUG
            if (sharpen::GetErrorCode(error) != sharpen::ErrorNotConnected) {
                assert("failed to write mail content to channel" && false);
            }
#endif
        }
#else
        size = channel->WriteFixedAsync(mail.Content());
#endif
        if (!size) {
            AbortConn(channel.get());
            return sharpen::Mail{};
        }
    }
    // receive mail
    sharpen::Mail response{this->DoReceive(std::move(channel))};
    return response;
}

sharpen::ActorId sharpen::TcpPoster::NviGetId() const noexcept {
    return this->remoteEndpoint_->GetActorId();
}

sharpen::TcpPoster::TcpPoster(std::unique_ptr<sharpen::IEndPoint> endpoint,
                              std::shared_ptr<sharpen::ITcpSteamFactory> factory)
    : Self{std::move(endpoint), std::move(factory), nullptr} {
}

sharpen::TcpPoster::TcpPoster(std::unique_ptr<sharpen::IEndPoint> endpoint,
                              std::shared_ptr<sharpen::ITcpSteamFactory> factory,
                              std::unique_ptr<sharpen::IWorkerGroup> pipelineWorker)
    : lock_(nullptr)
    , channel_(nullptr)
    , remoteEndpoint_(std::move(endpoint))
    , parser_(nullptr)
    , factory_(std::move(factory))
    , pipelineWorker_(std::move(pipelineWorker)) {
    assert(this->factory_);
    assert(this->remoteEndpoint_);
    this->lock_.reset(new (std::nothrow) sharpen::SpinLock{});
    if (!this->lock_) {
        throw std::bad_alloc{};
    }
}

sharpen::TcpPoster &sharpen::TcpPoster::operator=(Self &&other) noexcept {
    if (this != std::addressof(other)) {
        this->lock_ = std::move(other.lock_);
        this->channel_ = std::move(other.channel_);
        this->remoteEndpoint_ = std::move(other.remoteEndpoint_);
        this->parser_ = std::move(other.parser_);
        this->factory_ = std::move(other.factory_);
        this->pipelineWorker_ = std::move(other.pipelineWorker_);
    }
    return *this;
}