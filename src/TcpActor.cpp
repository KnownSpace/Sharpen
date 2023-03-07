#include <sharpen/TcpActor.hpp>

#include <new>

#include <sharpen/RemotePosterOpenError.hpp>
#include <sharpen/SingleWorkerGroup.hpp>
#include <sharpen/YieldOps.hpp>
#include <sharpen/SystemError.hpp>

void sharpen::TcpActor::DoReceive(sharpen::Mail response) noexcept
{
    if(!response.Empty())
    {
        sharpen::RemoteActorStatus expectedStatus{sharpen::RemoteActorStatus::InProgress};
        if(this->status_.compare_exchange_strong(expectedStatus,sharpen::RemoteActorStatus::Opened))
        {
            this->receiver_->Receive(std::move(response),this->GetId());
        }
    }
    else
    {
        sharpen::RemoteActorStatus status = sharpen::RemoteActorStatus::Closed;
        if(this->status_.exchange(status) != sharpen::RemoteActorStatus::Closed)
        {
            this->poster_->Close();
        }
    }
}

void sharpen::TcpActor::Receive(sharpen::AwaitableFuture<sharpen::Mail> *futurePtr) noexcept
{
    std::unique_ptr<sharpen::AwaitableFuture<sharpen::Mail>> future{futurePtr};
    sharpen::Mail response{future->Await()};
    this->DoReceive(response);
}

void sharpen::TcpActor::DoPostShared(const sharpen::Mail *mail) noexcept
{
    assert(mail);
    sharpen::RemoteActorStatus status{this->status_.exchange(sharpen::RemoteActorStatus::InProgress)};
    if(status == sharpen::RemoteActorStatus::Closed)
    {
        try
        {
            std::unique_ptr<sharpen::IMailParser> parser{this->parserFactory_->Produce()};
            this->poster_->Open(std::move(parser));
        }
        catch(const sharpen::RemotePosterOpenError &ignore)
        {
            (void)ignore;
            return;
        }
        catch(const std::system_error &error)
        {
            sharpen::ErrorCode errorCode{sharpen::GetErrorCode(error)};
            if(sharpen::IsFatalError(errorCode))
            {
                std::terminate();
            }
            assert(!error.what() && "fail to post mail");
            (void)error;
            return;
        }
        catch(const std::exception &ignore)
        {
            assert(!ignore.what() && "fail to post mail");
            (void)ignore;
            return;
        }
    }
    if(this->receiveWorker_)
    {
        std::unique_ptr<sharpen::AwaitableFuture<sharpen::Mail>> future;
        future.reset(new (std::nothrow) sharpen::AwaitableFuture<sharpen::Mail>{});
        if(!future)
        {
            //bad alloc
            std::terminate();
        }
        status = sharpen::RemoteActorStatus::Opened;
        if(!this->status_.compare_exchange_strong(status,sharpen::RemoteActorStatus::InProgress))
        {
            if(status == sharpen::RemoteActorStatus::Closed)
            {
                return;
            }
        }
        this->poster_->PostAsync(*future,*mail);
        this->receiveWorker_->Submit(&Self::Receive,this,future.release());
    }
    sharpen::Mail response{this->poster_->Post(*mail)};
    this->DoReceive(std::move(response));
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

void sharpen::TcpActor::NviPost(sharpen::Mail mail)
{
    this->postWorker_->Submit(&Self::DoPost,this,std::move(mail));
}

void sharpen::TcpActor::NviPostShared(const sharpen::Mail &mail)
{
    this->postWorker_->Submit(&Self::DoPostShared,this,&mail);
}

sharpen::TcpActor::TcpActor(sharpen::IFiberScheduler &scheduler,sharpen::IMailReceiver &receiver,std::shared_ptr<sharpen::IMailParserFactory> factory,std::unique_ptr<sharpen::IRemotePoster> poster)
    :Self{scheduler,receiver,std::move(factory),std::move(poster),false}
{}

sharpen::TcpActor::TcpActor(sharpen::IFiberScheduler &scheduler,sharpen::IMailReceiver &receiver,std::shared_ptr<sharpen::IMailParserFactory> factory,std::unique_ptr<sharpen::IRemotePoster> poster,bool enablePipeline)
    :receiver_(&receiver)
    ,poster_(std::move(poster))
    ,status_(sharpen::RemoteActorStatus::Closed)
    ,parserFactory_(std::move(factory))
    ,postWorker_(nullptr)
    ,receiveWorker_(nullptr)
{
    assert(this->parserFactory_);
    assert(this->poster_);
    sharpen::IWorkerGroup *worker{new (std::nothrow) sharpen::SingleWorkerGroup{scheduler}};
    if(!worker)
    {
        throw std::bad_alloc{};
    }
    this->postWorker_.reset(worker);
    if(enablePipeline)
    {
        worker = new (std::nothrow) sharpen::SingleWorkerGroup{scheduler};
        if(!worker)
        {
            throw std::bad_alloc{};
        }
        this->receiveWorker_.reset(worker);
    }
}

sharpen::TcpActor::~TcpActor() noexcept
{
    this->poster_->Close();
}