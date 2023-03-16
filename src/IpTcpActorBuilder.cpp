#include <sharpen/IpTcpActorBuilder.hpp>

#include <sharpen/IpTcpStreamFactory.hpp>
#include <sharpen/TcpPoster.hpp>
#include <sharpen/TcpActor.hpp>
#include <sharpen/SingleWorkerGroup.hpp>

sharpen::IpTcpActorBuilder::IpTcpActorBuilder()
    :Self{sharpen::IpEndPoint{0,0},false,sharpen::GetLocalScheduler(),sharpen::GetLocalLoopGroup()}
{}

sharpen::IpTcpActorBuilder::IpTcpActorBuilder(const sharpen::IpEndPoint &local)
    :Self{local,false,sharpen::GetLocalScheduler(),sharpen::GetLocalLoopGroup()}
{}

sharpen::IpTcpActorBuilder::IpTcpActorBuilder(bool pipeline)
    :Self{sharpen::IpEndPoint{0,0},pipeline,sharpen::GetLocalScheduler(),sharpen::GetLocalLoopGroup()}
{}

sharpen::IpTcpActorBuilder::IpTcpActorBuilder(const sharpen::IpEndPoint &local,bool pipeline)
    :Self{local,pipeline,sharpen::GetLocalScheduler(),sharpen::GetLocalLoopGroup()}
{}

sharpen::IpTcpActorBuilder::IpTcpActorBuilder(bool pipeline,sharpen::IFiberScheduler &scheduler,sharpen::IEventLoopGroup &loopGroup)
    :Self{sharpen::IpEndPoint{0,0},pipeline,scheduler,loopGroup}
{}

sharpen::IpTcpActorBuilder::IpTcpActorBuilder(const sharpen::IpEndPoint &local,bool pipeline,sharpen::IFiberScheduler &scheduler,sharpen::IEventLoopGroup &loopGroup)
    :remote_()
    ,scheduler_(&scheduler)
    ,factory_(nullptr)
    ,receiver_(nullptr)
    ,parserFactory_(nullptr)
    ,pipeline_(pipeline)
{
    this->factory_ = std::make_shared<sharpen::IpTcpStreamFactory>(loopGroup,local);
}

sharpen::IpTcpActorBuilder::IpTcpActorBuilder(Self &&other) noexcept
    :remote_(std::move(other.remote_))
    ,scheduler_(other.scheduler_)
    ,factory_(std::move(other.factory_))
    ,receiver_(other.receiver_)
    ,parserFactory_(std::move(other.parserFactory_))
    ,pipeline_(other.pipeline_)
{
    other.scheduler_ = nullptr;
    other.receiver_ = nullptr;
    other.pipeline_ = false;
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
        this->pipeline_ = other.pipeline_;
        other.scheduler_ = nullptr;
        other.receiver_ = nullptr;
        other.pipeline_ = false;
    }
    return *this;
}

void sharpen::IpTcpActorBuilder::PrepareRemote(const sharpen::IpEndPoint &remote) noexcept
{
    this->remote_ = remote;
}

void sharpen::IpTcpActorBuilder::PrepareReceiver(sharpen::IMailReceiver &receiver) noexcept
{
    this->receiver_ = &receiver;
}

void sharpen::IpTcpActorBuilder::PrepareParserFactory(std::shared_ptr<sharpen::IMailParserFactory> parserFactory) noexcept
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
    if(this->remote_.GetPort() == 0)
    {
        throw std::logic_error{"remote port could not be 0"};
    }
}

std::unique_ptr<sharpen::IRemoteActor> sharpen::IpTcpActorBuilder::NviBuild() const
{
    this->EnsureConfiguration();
    std::unique_ptr<sharpen::IEndPoint> remote{new (std::nothrow) sharpen::IpEndPoint{this->remote_}};
    if(!remote)
    {
        throw std::bad_alloc{};
    }
    std::unique_ptr<sharpen::IWorkerGroup> worker{nullptr};
    if(this->pipeline_)
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
    std::unique_ptr<sharpen::IRemoteActor> actor{new (std::nothrow) sharpen::TcpActor{*this->scheduler_,*this->receiver_,this->parserFactory_,std::move(poster),this->pipeline_}};
    if(!actor)
    {
        throw std::bad_alloc{};
    }
    return actor;
}

std::shared_ptr<sharpen::IRemoteActor> sharpen::IpTcpActorBuilder::NviBuildShared() const
{
    this->EnsureConfiguration();
    std::unique_ptr<sharpen::IEndPoint> remote{new (std::nothrow) sharpen::IpEndPoint{this->remote_}};
    if(!remote)
    {
        throw std::bad_alloc{};
    }
    std::unique_ptr<sharpen::IWorkerGroup> worker{nullptr};
    if(this->pipeline_)
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
    std::shared_ptr<sharpen::IRemoteActor> actor{std::make_shared<sharpen::TcpActor>(*this->scheduler_,*this->receiver_,this->parserFactory_,std::move(poster),this->pipeline_)};
    return actor;
}