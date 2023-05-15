#include <DropStep.hpp>

#include <sharpen/IpEndPoint.hpp>
#include <sharpen/DebugTools.hpp>

sharpen::HostPipelineResult DropStep::Consume(sharpen::INetStreamChannel &channel,
                                              const std::atomic_bool &active) noexcept {
    sharpen::ByteBuffer buf{4096};
    std::size_t size{channel.ReadAsync(buf)};
    sharpen::IpEndPoint remote;
    channel.GetRemoteEndPoint(remote);
    char remoteIp[25] = {0};
    remote.GetAddrString(remoteIp,sizeof(remoteIp));
    while (size != 0) {
        size = channel.ReadAsync(buf);
        (void)buf;
    }
    sharpen::SyncPrintf("%s:%u Leave\n",remoteIp,remote.GetPort());
    return sharpen::HostPipelineResult::Broken;
}