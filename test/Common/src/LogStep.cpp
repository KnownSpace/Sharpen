#include <common/LogStep.hpp>

#include <sharpen/DebugTools.hpp>
#include <sharpen/IpEndPoint.hpp>

sharpen::HostPipelineResult LogStep::Consume(sharpen::INetStreamChannel &channel,
                                             const std::atomic_bool &active) noexcept {
    sharpen::IpEndPoint remote;
    sharpen::IpEndPoint local;
    channel.GetRemoteEndPoint(remote);
    channel.GetLocalEndPoint(local);
    char remoteIp[25] = {0};
    char localIp[25] = {0};
    remote.GetAddrString(remoteIp, sizeof(remoteIp));
    local.GetAddrString(localIp, sizeof(localIp));
    sharpen::SyncPrintf(
        "New Conn:%s:%u->%s:%u\n", remoteIp, remote.GetPort(), localIp, local.GetPort());
    return sharpen::HostPipelineResult::Continue;
}