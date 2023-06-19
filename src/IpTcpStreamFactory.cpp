#include <sharpen/IpTcpStreamFactory.hpp>

sharpen::IpTcpStreamFactory::IpTcpStreamFactory(const sharpen::IpEndPoint &endpoint)
    : Self{sharpen::GetLocalLoopGroup(), endpoint} {
}

sharpen::IpTcpStreamFactory::IpTcpStreamFactory(sharpen::IEventLoopGroup &loopGroup,
                                                const sharpen::IpEndPoint &endpoint) noexcept
    : loopGroup_(&loopGroup)
    , localEndpoint_(endpoint) {
}

sharpen::NetStreamChannelPtr sharpen::IpTcpStreamFactory::NviProduce(sharpen::TcpStreamOption opt) {
    assert(this->loopGroup_);
    sharpen::NetStreamChannelPtr channel{sharpen::OpenTcpChannel(sharpen::AddressFamily::Ip)};
    if (opt.IsEnableReuseAddress()) {
        channel->SetReuseAddress(true);
    }
    channel->Bind(this->localEndpoint_);
    channel->Register(*this->loopGroup_);
    return channel;
}