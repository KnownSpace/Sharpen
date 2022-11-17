#include <sharpen/NetStreamActor.hpp>

void sharpen::NetStreamActor::DoClose() noexcept
{
    sharpen::NetStreamChannelPtr channel{nullptr};
    {
        std::unique_lock<sharpen::SpinLock> lock{this->lock_};
        std::swap(channel,this->channel_);
    }
    if(channel)
    {
        channel->Close();
    }
}

void sharpen::NetStreamActor::DoOpen()
{
    sharpen::NetStreamChannelPtr channel{nullptr};
    {
        std::unique_lock<sharpen::SpinLock> lock{this->lock_};
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

std::unique_ptr<sharpen::IMail> sharpen::NetStreamActor::DoPost(const sharpen::IMail &mail)
{
    sharpen::NetStreamChannelPtr channel{nullptr};
    {
        std::unique_lock<sharpen::SpinLock> lock{this->lock_};
        channel = this->channel_;
    }
    if(!channel)
    {
        //already closed
        throw sharpen::ActorClosedError{"actor already closed"};
    }
    //post mail
    sharpen::ByteBuffer header{mail.GenerateHeader()};
    if(!header.Empty())
    {
        channel->WriteAsync(header);
    }
    const sharpen::IMailContent &content{mail.Content()};
    if(content.GetSize())
    {
        channel->WriteAsync(content.Content(),content.GetSize());
    }
    //receive mail
    auto response{this->ReceiveMail(this->channel_.get())};
    return response;
}