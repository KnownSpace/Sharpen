#include <sharpen/TcpPoster.hpp>

#include <new>
#include <cassert>

#include <sharpen/SystemError.hpp>

void sharpen::TcpPoster::NviClose() noexcept
{
    sharpen::NetStreamChannelPtr channel{nullptr};
    {
        assert(this->lock_);
        std::unique_lock<sharpen::SpinLock> lock{*this->lock_};
        std::swap(channel,this->channel_);
    }
    if(channel)
    {
        channel->Close();
    }
}

void sharpen::TcpPoster::NviOpen(std::unique_ptr<sharpen::IMailParser> parser)
{
    this->parser_ = std::move(parser);
    sharpen::NetStreamChannelPtr channel{nullptr};
    {
        assert(this->lock_);
        std::unique_lock<sharpen::SpinLock> lock{*this->lock_};
        if(!this->channel_)
        {
            assert(this->factory_);
            this->channel_ = this->factory_->Produce();
        }
        channel = this->channel_;
    }
    assert(channel);
    try
    {
        channel->ConnectAsync(*this->remoteEndpoint_);
    }
    catch(const std::system_error &error)
    {
        sharpen::ErrorCode errorCode{sharpen::GetErrorCode(error)};
        switch (errorCode)
        {
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
        }
        throw;
    }
    catch(const std::exception &rethrow)
    {
        (void)rethrow;
        throw;
    }
}

sharpen::Mail sharpen::TcpPoster::Receive(sharpen::NetStreamChannelPtr channel)
{
    std::size_t size{0};
    sharpen::ByteBuffer buffer{4096};
    sharpen::Mail response;
    while (!this->parser_->Completed())
    {
        size = channel->ReadAsync(buffer);
        if(!size)
        {
            return sharpen::Mail{};
        }
        sharpen::ByteSlice slice{buffer.Data(),size};
        this->parser_->Parse(slice);
    }
    response = this->parser_->PopCompletedMail();
    return response;
}

void sharpen::TcpPoster::NviPostAsync(sharpen::Future<sharpen::Mail> &future,const sharpen::Mail &mail) noexcept
{
    sharpen::NetStreamChannelPtr channel{nullptr};
    {
        assert(this->lock_);
        std::unique_lock<sharpen::SpinLock> lock{*this->lock_};
        channel = this->channel_;
    }
    if(!channel)
    {
        future.Complete();
        return;
    }
    //post mail
    std::size_t size{0};
    size = channel->WriteAsync(mail.Header());
    if(!size)
    {
        future.Complete();
        return;
    }
    if(!mail.Content().Empty())
    {
        size = channel->WriteAsync(mail.Content());
        if(!size)
        {
            future.Complete();
            return;
        }
    }
    //receive mail
    if(!this->pipelineWorker_)
    {
        sharpen::Mail response{this->Receive(std::move(channel))};
        future.Complete(std::move(response));
        return;
    }
    //pipeline model
    this->pipelineWorker_->Invoke(future,&Self::Receive,this,std::move(channel));
}

sharpen::Mail sharpen::TcpPoster::NviPost(const sharpen::Mail &mail) noexcept
{
    sharpen::NetStreamChannelPtr channel{nullptr};
    {
        assert(this->lock_);
        std::unique_lock<sharpen::SpinLock> lock{*this->lock_};
        channel = this->channel_;
    }
    if(!channel)
    {
        return sharpen::Mail{};
    }
    //post mail
    std::size_t size{0};
    size = channel->WriteAsync(mail.Header());
    if(!size)
    {
        return sharpen::Mail{};
    }
    if(!mail.Content().Empty())
    {
        size = channel->WriteAsync(mail.Content());
        if(!size)
        {
            return sharpen::Mail{};
        }
    }
    //receive mail
    sharpen::Mail response{this->Receive(std::move(channel))};
    return response;
}

std::uint64_t sharpen::TcpPoster::NviGetId() const noexcept
{
    return this->remoteEndpoint_->GetActorId();
}

sharpen::TcpPoster::TcpPoster(std::unique_ptr<sharpen::IEndPoint> endpoint,std::shared_ptr<sharpen::ITcpSteamFactory> factory)
    :Self{std::move(endpoint),std::move(factory),nullptr}
{}

sharpen::TcpPoster::TcpPoster(std::unique_ptr<sharpen::IEndPoint> endpoint,std::shared_ptr<sharpen::ITcpSteamFactory> factory,std::unique_ptr<sharpen::IWorkerGroup> worker)
    :lock_(nullptr)
    ,channel_(nullptr)
    ,remoteEndpoint_(std::move(endpoint))
    ,parser_(nullptr)
    ,factory_(std::move(factory))
    ,pipelineWorker_(std::move(worker))
{
    assert(this->factory_);
    assert(this->remoteEndpoint_);
    this->lock_.reset(new (std::nothrow) sharpen::SpinLock{});
    if(!this->lock_)
    {
        throw std::bad_alloc{};
    }
}

sharpen::TcpPoster &sharpen::TcpPoster::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->lock_ = std::move(other.lock_);
        this->channel_ = std::move(other.channel_);
        this->remoteEndpoint_ = std::move(other.remoteEndpoint_);
        this->parser_ = std::move(other.parser_);
        this->factory_ = std::move(other.factory_);
    }
    return *this;
}