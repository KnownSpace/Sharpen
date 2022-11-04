#include <sharpen/RemoteActorOperator.hpp>

void sharpen::RemoteActorOperator::DoPost(sharpen::Future<bool> *future,const sharpen::IMail *mail) noexcept
{
    try
    {
        if(!this->isOpened_)
        {
            this->actor_->Open();
            this->isOpened_ = true;
        }
        bool result{this->actor_->Post(*mail)};
        future->Complete(result);
    }
    catch(const std::exception &)
    {
        future->Fail(std::current_exception());
    }
}

void sharpen::RemoteActorOperator::PostAsync(sharpen::Future<bool> &future,const sharpen::IMail &mail)
{
    this->worker_.Submit(&Self::DoPost,this,&future,&mail);
}

