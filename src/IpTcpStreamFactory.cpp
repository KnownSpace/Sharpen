#include <sharpen/IpTcpStreamFactory.hpp>

sharpen::IpTcpStreamFactory::IpTcpStreamFactory(sharpen::IEventLoopGroup &loopGroup,sharpen::IpEndPoint endpoint)
    :loopGroup_(&loopGroup)
    ,localEndpoint_(std::move(endpoint))
{}

sharpen::NetStreamChannelPtr sharpen::IpTcpStreamFactory::NviProduce()
{
    assert(this->loopGroup_);
    sharpen::NetStreamChannelPtr channel{sharpen::OpenTcpStreamChannel(sharpen::AddressFamily::Ip)};
    channel->Bind(this->localEndpoint_);
    channel->Register(*this->loopGroup_);
    return channel;
}