#include <sharpen/IpTcpActorBuilder.hpp>

#include <sharpen/IpTcpStreamFactory.hpp>
#include <sharpen/TcpPoster.hpp>
#include <sharpen/TcpActor.hpp>

sharpen::IpTcpActorBuilder::IpTcpActorBuilder(sharpen::IFiberScheduler &scheduler,sharpen::IEventLoopGroup &loopGroup)
    :remote_()
    ,scheduler_(&scheduler)
    ,factory_(nullptr)
    ,receiver_(nullptr)
    ,parserFactory_(nullptr)
{
    sharpen::IpEndPoint local;
    local.SetAddrByString("0.0.0.0");
    local.SetPort(0);
    this->factory_ = std::make_shared<sharpen::IpTcpStreamFactory>(loopGroup,local);
}

sharpen::IpTcpActorBuilder::IpTcpActorBuilder(Self &&other) noexcept
    :remote_(std::move(other.remote_))
    ,scheduler_(other.scheduler_)
    ,factory_(std::move(other.factory_))
    ,receiver_(other.receiver_)
    ,parserFactory_(std::move(other.parserFactory_))
{
    other.scheduler_ = nullptr;
    other.receiver_ = nullptr;
}

sharpen::IpTcpActorBuilder &sharpen::IpTcpActorBuilder::operator=(Self &&other) noexcept
{
    if(this != std::addressof(other))
    {
        this->remote_ = std::move(other.remote_);
        this->scheduler_ = other.scheduler_;
        this->factory_ = std::move(other.factory_);
        this->receiver_ = other.receiver_;
        this->parserFactory_ = std::move(other.parserFactory_);
        other.scheduler_ = nullptr;
        other.receiver_ = nullptr;
    }
    return *this;
}

void sharpen::IpTcpActorBuilder::SetRemote(const sharpen::IpEndPoint &remote) noexcept
{
    this->remote_ = remote;
}

void sharpen::IpTcpActorBuilder::SetReceiver(sharpen::IMailReceiver &receiver) noexcept
{
    this->receiver_ = &receiver;
}

void sharpen::IpTcpActorBuilder::SetParserFactory(std::shared_ptr<sharpen::IMailParserFactory> parserFactory) noexcept
{
    this->parserFactory_ = std::move(parserFactory);
}

void sharpen::IpTcpActorBuilder::EnsureConfiguration() const
{
    assert(this->scheduler_);
    assert(this->factory_);
    if(!this->receiver_)
    {
        throw std::logic_error{"receiver could not be null"};
    }
    if(!this->parserFactory_)
    {
        throw std::logic_error{"parser factory could not be null"};
    }
}

std::unique_ptr<sharpen::IRemoteActor> sharpen::IpTcpActorBuilder::Build() const
{
    this->EnsureConfiguration();
    std::unique_ptr<sharpen::IEndPoint> remote{new (std::nothrow) sharpen::IpEndPoint{this->remote_}};
    if(!remote)
    {
        throw std::bad_alloc{};
    }
    std::unique_ptr<sharpen::IRemotePoster> poster{new (std::nothrow) sharpen::TcpPoster{std::move(remote),this->factory_}};
    if(!poster)
    {
        throw std::bad_alloc{};
    }
    std::unique_ptr<sharpen::IRemoteActor> actor{new (std::nothrow) sharpen::TcpActor{*this->scheduler_,*this->receiver_,this->parserFactory_,std::move(poster)}};
    if(!actor)
    {
        throw std::bad_alloc{};
    }
    return actor;
}

std::shared_ptr<sharpen::IRemoteActor> sharpen::IpTcpActorBuilder::BuildShared() const
{
    this->EnsureConfiguration();
    std::unique_ptr<sharpen::IEndPoint> remote{new (std::nothrow) sharpen::IpEndPoint{this->remote_}};
    if(!remote)
    {
        throw std::bad_alloc{};
    }
    std::unique_ptr<sharpen::IRemotePoster> poster{new (std::nothrow) sharpen::TcpPoster{std::move(remote),this->factory_}};
    if(!poster)
    {
        throw std::bad_alloc{};
    }
    std::shared_ptr<sharpen::IRemoteActor> actor{std::make_shared<sharpen::TcpActor>(*this->scheduler_,*this->receiver_,this->parserFactory_,std::move(poster))};
    return actor;
}