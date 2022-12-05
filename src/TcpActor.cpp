#include <sharpen/TcpActor.hpp>

#include <new>

#include <sharpen/RemotePosterOpenError.hpp>
#include <sharpen/RemotePosterClosedError.hpp>
#include <sharpen/SingleWorkerGroup.hpp>

void sharpen::TcpActor::DoPostShared(const sharpen::Mail *mail) noexcept
{
    assert(mail);
    sharpen::RemoteActorStatus status{this->status_.exchange(sharpen::RemoteActorStatus::InProgress)};
    if(status == sharpen::RemoteActorStatus::Closed)
    {
        try
        {
            this->poster_->Open();
        }
        catch(const sharpen::RemotePosterOpenError &ignore)
        {
            (void)ignore;
            this->status_ = sharpen::RemoteActorStatus::Closed;
            return;
        }
        catch(const std::exception &ignore)
        {
            assert(!ignore.what() && "fail to post mail");
            (void)ignore;
        }
    }
    sharpen::Mail response{};
    try
    {
        response = this->poster_->Post(*mail);
        this->receiver_->ReceiveMail(std::move(response));
    }
    catch(const sharpen::RemotePosterClosedError &ignore)
    {
        //cancel by operator
        (void)ignore;
        this->status_ = sharpen::RemoteActorStatus::Closed;
        return;
    }
    catch(const std::exception &ignore)
    {
        assert(!ignore.what() && "fail to post mail or receive");
        (void)ignore;
    }
    this->status_ = sharpen::RemoteActorStatus::Opened;
}

void sharpen::TcpActor::DoPost(sharpen::Mail mail) noexcept
{
    this->DoPostShared(&mail);
}

void sharpen::TcpActor::Cancel() noexcept
{
    if(this->status_.exchange(sharpen::RemoteActorStatus::Closed) != sharpen::RemoteActorStatus::Closed)
    {
        this->Cancel();
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

sharpen::TcpActor::TcpActor(sharpen::IFiberScheduler &scheduler,sharpen::IMailReceiver &receiver,std::unique_ptr<sharpen::IRemotePoster> poster)
    :receiver_(&receiver)
    ,poster_(std::move(poster))
    ,status_(sharpen::RemoteActorStatus::Closed)
    ,worker_(nullptr)
{
    sharpen::IWorkerGroup *worker{new (std::nothrow) sharpen::SingleWorkerGroup{scheduler}};
    if(!worker)
    {
        throw std::bad_alloc{};
    }
    this->worker_.reset(worker);
}

