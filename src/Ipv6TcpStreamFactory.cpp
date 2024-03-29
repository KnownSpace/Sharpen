#include <sharpen/Ipv6TcpStreamFactory.hpp>

sharpen::Ipv6TcpStreamFactory::Ipv6TcpStreamFactory(const sharpen::Ipv6EndPoint &endpoint)
    : Self{sharpen::GetLocalLoopGroup(), endpoint} {
}

sharpen::Ipv6TcpStreamFactory::Ipv6TcpStreamFactory(sharpen::IEventLoopGroup &loopGroup,
                                                    const sharpen::Ipv6EndPoint &endpoint) noexcept
    : loopGroup_(&loopGroup)
    , localEndpoint_(endpoint) {
}

sharpen::NetStreamChannelPtr sharpen::Ipv6TcpStreamFactory::NviProduce(sharpen::TcpStreamOption opt) {
    assert(this->loopGroup_);
    sharpen::NetStreamChannelPtr channel{sharpen::OpenTcpChannel(sharpen::AddressFamily::Ipv6)};
    if (opt.IsEnableReuseAddress()) {
        channel->SetReuseAddress(true);
    }
    channel->Bind(this->localEndpoint_);
    channel->Register(*this->loopGroup_);
    return channel;
}