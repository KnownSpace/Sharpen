#include <sharpen/TcpServer.hpp>

sharpen::TcpServer::TcpServer(sharpen::AddressFamily af,const sharpen::IEndPoint &endpoint,sharpen::EventEngine &engine)
    :acceptor_(af,endpoint,engine)
    ,running_(true)
    ,waiter_()
    ,engine_(&engine)
{}

void sharpen::TcpServer::RunAsync()
{
    while (this->running_)
    {
        try
        {
            sharpen::NetStreamChannelPtr channel = this->acceptor_.AcceptAsync();
            channel->Register(*this->engine_);
            this->engine_->Launch(&sharpen::TcpServer::OnNewChannel,this,channel);
        }
        catch(const std::exception&)
        {
            return;
        }
    }
}

void sharpen::TcpServer::Stop() noexcept
{
    this->running_ = false;
    this->acceptor_.Close();
}