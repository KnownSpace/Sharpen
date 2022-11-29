#include <sharpen/TcpActor.hpp>

#include <new>
#include <cassert>

void sharpen::TcpActor::DoClose() noexcept
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

void sharpen::TcpActor::DoOpen()
{
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
    channel->ConnectAsync(*this->remoteEndpoint_);
}

sharpen::Mail sharpen::TcpActor::DoPost(const sharpen::Mail &mail)
{
    sharpen::NetStreamChannelPtr channel{nullptr};
    {
        assert(this->lock_);
        std::unique_lock<sharpen::SpinLock> lock{*this->lock_};
        channel = this->channel_;
    }
    if(!channel)
    {
        //already closed
        throw sharpen::RemoteActorClosedError{"actor already closed"};
    }
    //post mail
    if(!mail.Header().Empty())
    {
        channel->WriteAsync(mail.Header());
    }
    if(!mail.Content().Empty())
    {
        channel->WriteAsync(mail.Content());
    }
    //receive mail
    sharpen::ByteBuffer buffer{4096};
    sharpen::Mail response;
    while (!this->parser_->Completed())
    {
        std::size_t sz{channel->ReadAsync(buffer)};
        if(!sz)
        {
            throw sharpen::RemoteActorClosedError{"actor already closed"};
        }
        sharpen::ByteSlice slice{buffer.Data(),sz};
        this->parser_->Parse(slice);
    }
    response = this->parser_->PopCompletedMail();
    return response;
}

std::uint64_t sharpen::TcpActor::DoGetId() const noexcept
{
    return this->remoteEndpoint_->GetHashCode64();
}

sharpen::TcpActor::TcpActor(std::unique_ptr<sharpen::IEndPoint> endpoint,std::unique_ptr<sharpen::IMailParser> parser,sharpen::INetSteamFactory *factory)
    :lock_(nullptr)
    ,channel_(nullptr)
    ,remoteEndpoint_(std::move(endpoint))
    ,parser_(std::move(parser))
    ,factory_(factory)
{
    assert(this->remoteEndpoint_);
    assert(this->parser_);
    assert(this->factory_);
    this->lock_.reset(new (std::nothrow) sharpen::SpinLock{});
    if(!this->lock_)
    {
        throw std::bad_alloc{};
    }
}

sharpen::TcpActor::TcpActor(Self &&other) noexcept
    :lock_(std::move(other.lock_))
    ,channel_(std::move(other.channel_))
    ,remoteEndpoint_(std::move(other.remoteEndpoint_))
    ,parser_(std::move(other.parser_))
    ,factory_(other.factory_)
{
    other.factory_ = nullptr;
}

sharpen::TcpActor &sharpen::TcpActor::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->lock_ = std::move(other.lock_);
        this->channel_ = std::move(other.channel_);
        this->remoteEndpoint_ = std::move(other.remoteEndpoint_);
        this->parser_ = std::move(other.parser_);
        this->factory_ = other.factory_;
        other.factory_ = nullptr;
    }
    return *this;
}