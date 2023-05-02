#include <sharpen/IpTcpStreamFactory.hpp>

sharpen::IpTcpStreamFactory::IpTcpStreamFactory(const sharpen::IpEndPoint &endpoint)
    : Self{sharpen::GetLocalLoopGroup(), endpoint} {
}

sharpen::IpTcpStreamFactory::IpTcpStreamFactory(sharpen::IEventLoopGroup &loopGroup,
                                                const sharpen::IpEndPoint &endpoint) noexcept
    : loopGroup_(&loopGroup)
    , localEndpoint_(endpoint) {
}

sharpen::NetStreamChannelPtr sharpen::IpTcpStreamFactory::NviProduce() {
    assert(this->loopGroup_);
    sharpen::NetStreamChannelPtr channel{sharpen::OpenTcpChannel(sharpen::AddressFamily::Ip)};
    channel->Bind(this->localEndpoint_);
    channel->Register(*this->loopGroup_);
    return channel;
}