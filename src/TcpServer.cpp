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
        sharpen::NetStreamChannelPtr channel = this->acceptor_.AcceptAsync();
        channel->Register(*this->engine_);
        this->engine_->Launch(&sharpen::TcpServer::OnNewChannel,this,channel);
    }
    this->waiter_.Complete();
}

void sharpen::TcpServer::StartAsync()
{
    this->engine_->Launch(&sharpen::TcpServer::RunAsync,this);
}

void sharpen::TcpServer::Stop() noexcept
{
    this->running_ = false;
    this->acceptor_.Close();
}

void sharpen::TcpServer::Await() noexcept
{
    this->waiter_.Await();
}