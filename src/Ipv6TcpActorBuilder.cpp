#include <sharpen/Ipv6TcpActorBuilder.hpp>

#include <sharpen/Ipv6TcpStreamFactory.hpp>
#include <sharpen/TcpPoster.hpp>
#include <sharpen/TcpActor.hpp>
#include <sharpen/SingleWorkerGroup.hpp>

sharpen::Ipv6TcpActorBuilder::Ipv6TcpActorBuilder()
    :Self{sharpen::GetLocalScheduler(),sharpen::GetLocalLoopGroup()}
{}

sharpen::Ipv6TcpActorBuilder::Ipv6TcpActorBuilder(const sharpen::Ipv6EndPoint &local)
    :Self{local,sharpen::GetLocalScheduler(),sharpen::GetLocalLoopGroup()}
{}

sharpen::Ipv6TcpActorBuilder::Ipv6TcpActorBuilder(sharpen::IFiberScheduler &scheduler,sharpen::IEventLoopGroup &loopGroup)
    :remote_()
    ,scheduler_(&scheduler)
    ,factory_(nullptr)
    ,receiver_(nullptr)
    ,parserFactory_(nullptr)
{
    sharpen::Ipv6EndPoint local;
    in6_addr addr;
    std::memset(&addr,0,sizeof(addr));
    local.SetAddr(addr);
    local.SetPort(0);
    this->factory_ = std::make_shared<sharpen::Ipv6TcpStreamFactory>(loopGroup,local);
}

sharpen::Ipv6TcpActorBuilder::Ipv6TcpActorBuilder(const sharpen::Ipv6EndPoint &local,sharpen::IFiberScheduler &scheduler,sharpen::IEventLoopGroup &loopGroup)
    :remote_()
    ,scheduler_(&scheduler)
    ,factory_(nullptr)
    ,receiver_(nullptr)
    ,parserFactory_(nullptr)
{
    this->factory_ = std::make_shared<sharpen::Ipv6TcpStreamFactory>(loopGroup,local);
}

sharpen::Ipv6TcpActorBuilder::Ipv6TcpActorBuilder(Self &&other) noexcept
    :remote_(std::move(other.remote_))
    ,scheduler_(other.scheduler_)
    ,factory_(std::move(other.factory_))
    ,receiver_(other.receiver_)
    ,parserFactory_(std::move(other.parserFactory_))
{
    other.scheduler_ = nullptr;
    other.receiver_ = nullptr;
}

sharpen::Ipv6TcpActorBuilder &sharpen::Ipv6TcpActorBuilder::operator=(Self &&other) noexcept
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

void sharpen::Ipv6TcpActorBuilder::PrepareRemote(const sharpen::Ipv6EndPoint &remote) noexcept
{
    this->remote_ = remote;
}

void sharpen::Ipv6TcpActorBuilder::PrepareReceiver(sharpen::IMailReceiver &receiver) noexcept
{
    this->receiver_ = &receiver;
}

void sharpen::Ipv6TcpActorBuilder::PrepareParserFactory(std::shared_ptr<sharpen::IMailParserFactory> parserFactory) noexcept
{
    this->parserFactory_ = std::move(parserFactory);
}

void sharpen::Ipv6TcpActorBuilder::EnsureConfiguration() const
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
    if(this->remote_.GetPort() == 0)
    {
        throw std::logic_error{"remote port could not be 0"};
    }
}

std::unique_ptr<sharpen::IRemoteActor> sharpen::Ipv6TcpActorBuilder::NviBuild(bool pipeline) const
{
    this->EnsureConfiguration();
    std::unique_ptr<sharpen::IEndPoint> remote{new (std::nothrow) sharpen::Ipv6EndPoint{this->remote_}};
    if(!remote)
    {
        throw std::bad_alloc{};
    }
    std::unique_ptr<sharpen::IWorkerGroup> worker{nullptr};
    if(pipeline)
    {
        worker.reset(new (std::nothrow) sharpen::SingleWorkerGroup{*this->scheduler_});
        if(!worker)
        {
            throw std::bad_alloc{};
        }
    }
    std::unique_ptr<sharpen::IRemotePoster> poster{new (std::nothrow) sharpen::TcpPoster{std::move(remote),this->factory_,std::move(worker)}};
    if(!poster)
    {
        throw std::bad_alloc{};
    }
    std::unique_ptr<sharpen::IRemoteActor> actor{new (std::nothrow) sharpen::TcpActor{*this->scheduler_,*this->receiver_,this->parserFactory_,std::move(poster),pipeline}};
    if(!actor)
    {
        throw std::bad_alloc{};
    }
    return actor;
}

std::shared_ptr<sharpen::IRemoteActor> sharpen::Ipv6TcpActorBuilder::NviBuildShared(bool pipeline) const
{
    this->EnsureConfiguration();
    std::unique_ptr<sharpen::IEndPoint> remote{new (std::nothrow) sharpen::Ipv6EndPoint{this->remote_}};
    if(!remote)
    {
        throw std::bad_alloc{};
    }
    std::unique_ptr<sharpen::IWorkerGroup> worker{nullptr};
    if(pipeline)
    {
        worker.reset(new (std::nothrow) sharpen::SingleWorkerGroup{*this->scheduler_});
        if(!worker)
        {
            throw std::bad_alloc{};
        }
    }
    std::unique_ptr<sharpen::IRemotePoster> poster{new (std::nothrow) sharpen::TcpPoster{std::move(remote),this->factory_,std::move(worker)}};
    if(!poster)
    {
        throw std::bad_alloc{};
    }
    std::shared_ptr<sharpen::IRemoteActor> actor{std::make_shared<sharpen::TcpActor>(*this->scheduler_,*this->receiver_,this->parserFactory_,std::move(poster),pipeline)};
    return actor;
}