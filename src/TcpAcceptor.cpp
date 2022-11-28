#include <sharpen/TcpAcceptor.hpp>

sharpen::TcpAcceptor::TcpAcceptor(sharpen::AddressFamily af,const sharpen::IEndPoint &endpoint,sharpen::EventEngine &engine)
    :listener_()
{
    this->listener_ = sharpen::OpenTcpStreamChannel(af);
    this->listener_->Register(engine);
#ifdef SHARPEN_IS_NIX
    this->listener_->SetReuseAddress(true);
#endif
    this->listener_->Bind(endpoint);
    this->listener_->Listen(65535);
}

sharpen::NetStreamChannelPtr sharpen::TcpAcceptor::AcceptAsync()
{
    return this->listener_->AcceptAsync();
}