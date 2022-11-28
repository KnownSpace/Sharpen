#include <sharpen/IpNetStreamFactory.hpp>

sharpen::NetStreamChannelPtr sharpen::IpNetStreamFactory::DoProduce()
{
    assert(this->engine_);
    sharpen::NetStreamChannelPtr channel{sharpen::OpenTcpStreamChannel(sharpen::AddressFamily::Ip)};
    channel->Bind(this->localEndpoint_);
    channel->Register(*this->engine_);
    return channel;
}