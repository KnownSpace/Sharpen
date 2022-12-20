#include <sharpen/Ipv6TcpStreamFactory.hpp>

sharpen::Ipv6TcpStreamFactory::Ipv6TcpStreamFactory(sharpen::IEventLoopGroup &loopGroup,sharpen::Ipv6EndPoint endpoint)
    :loopGroup_(&loopGroup)
    ,localEndpoint_(std::move(endpoint))
{}

sharpen::NetStreamChannelPtr sharpen::Ipv6TcpStreamFactory::NviProduce()
{
    assert(this->loopGroup_);
    sharpen::NetStreamChannelPtr channel{sharpen::OpenTcpStreamChannel(sharpen::AddressFamily::Ipv6)};
    channel->Bind(this->localEndpoint_);
    channel->Register(*this->loopGroup_);
    return channel;
}