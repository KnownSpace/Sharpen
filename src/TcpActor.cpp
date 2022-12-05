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

void sharpen::TcpActor::DoCancel(sharpen::Future<void> *future) noexcept
{
    future->Complete();
}

void sharpen::TcpActor::Cancel() noexcept
{
    sharpen::RemoteActorStatus status{sharpen::RemoteActorStatus::InProgress};
    while(!this->status_.compare_exchange_weak(status,sharpen::RemoteActorStatus::Closed))
    {
        if(status != sharpen::RemoteActorStatus::InProgress)
        {
            break;
        }
    }
    if(status == sharpen::RemoteActorStatus::InProgress)
    {
        using FnPtr = void(*)(sharpen::Future<void>*);
        this->poster_->Close();
        sharpen::AwaitableFuture<void> future;
        this->worker_->Submit(static_cast<FnPtr>(&Self::DoCancel),&future);
        future.WaitAsync();
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

