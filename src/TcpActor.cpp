#include <sharpen/TcpActor.hpp>

#include <new>

#include <sharpen/RemotePosterOpenError.hpp>
#include <sharpen/RemotePosterClosedError.hpp>
#include <sharpen/SingleWorkerGroup.hpp>
#include <sharpen/YieldOps.hpp>

void sharpen::TcpActor::DoPostShared(const sharpen::Mail *mail) noexcept
{
    assert(mail);
    sharpen::RemoteActorStatus status{this->status_.exchange(sharpen::RemoteActorStatus::InProgress)};
    if(status == sharpen::RemoteActorStatus::Closed)
    {
        try
        {
            this->poster_->Open(this->parserFactory_->Produce());
        }
        catch(const sharpen::RemotePosterOpenError &ignore)
        {
            (void)ignore;
            return;
        }
        catch(const std::exception &ignore)
        {
            assert(!ignore.what() && "fail to post mail");
            (void)ignore;
        }
    }
    sharpen::Mail response;
    try
    {
        response = this->poster_->Post(*mail);
        sharpen::RemoteActorStatus expectedStatus{sharpen::RemoteActorStatus::InProgress};
        if(this->status_.compare_exchange_strong(expectedStatus,sharpen::RemoteActorStatus::Opened))
        {
            this->receiver_->Receive(std::move(response),this->GetId());
        }
    }
    catch(const sharpen::RemotePosterClosedError &ignore)
    {
        //cancel by operator
        (void)ignore;
        return;
    }
    catch(const std::exception &ignore)
    {
        assert(!ignore.what() && "fail to post or receive mail");
        (void)ignore;
    }
}

void sharpen::TcpActor::DoPost(sharpen::Mail mail) noexcept
{
    this->DoPostShared(&mail);
}

void sharpen::TcpActor::Cancel() noexcept
{
    sharpen::RemoteActorStatus expectedStatus{sharpen::RemoteActorStatus::InProgress};
    if(this->status_.compare_exchange_strong(expectedStatus,sharpen::RemoteActorStatus::Closed))
    {
        this->poster_->Close();
    }
}

void sharpen::TcpActor::Post(sharpen::Mail mail)
{
    this->worker_->Submit(&Self::DoPost,this,std::move(mail));
}

void sharpen::TcpActor::PostShared(const sharpen::Mail &mail)
{
    this->worker_->Submit(&Self::DoPostShared,this,&mail);
}

sharpen::TcpActor::TcpActor(sharpen::IFiberScheduler &scheduler,sharpen::IMailReceiver &receiver,std::shared_ptr<sharpen::IMailParserFactory> factory,std::unique_ptr<sharpen::IRemotePoster> poster)
    :receiver_(&receiver)
    ,poster_(std::move(poster))
    ,status_(sharpen::RemoteActorStatus::Closed)
    ,worker_(nullptr)
    ,parserFactory_(std::move(factory))
{
    assert(this->parserFactory_);
    assert(this->poster_);
    sharpen::IWorkerGroup *worker{new (std::nothrow) sharpen::SingleWorkerGroup{scheduler}};
    if(!worker)
    {
        throw std::bad_alloc{};
    }
    this->worker_.reset(worker);
}

sharpen::TcpActor::~TcpActor() noexcept
{
    this->poster_->Close();
}