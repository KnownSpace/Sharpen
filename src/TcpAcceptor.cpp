#include <sharpen/TcpAcceptor.hpp>

sharpen::TcpAcceptor::TcpAcceptor(sharpen::AddressFamily af,const sharpen::IEndPoint &endpoint,sharpen::EventEngine &engine)
    :listener_()
{
    this->listener_ = sharpen::MakeTcpStreamChannel(af);
    this->listener_->Register(engine);
    this->listener_->Bind(endpoint);
    this->listener_->Listen(65535);
}

sharpen::NetStreamChannelPtr sharpen::TcpAcceptor::AcceptAsync()
{
    return this->listener_->AcceptAsync();
}