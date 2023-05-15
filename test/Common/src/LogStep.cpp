#include <LogStep.hpp>

#include <sharpen/DebugTools.hpp>
#include <sharpen/IpEndPoint.hpp>

sharpen::HostPipelineResult LogStep::Consume(sharpen::INetStreamChannel &channel,
                                             const std::atomic_bool &active) noexcept {
    sharpen::IpEndPoint remote;
    channel.GetRemoteEndPoint(remote);
    sharpen::IpEndPoint local;
    channel.GetLocalEndPoint(local);
    char remoteIp[25] = {0};
    char localIp[25] = {0};
    remote.GetAddrString(remoteIp,sizeof(remoteIp));
    local.GetAddrString(localIp,sizeof(localIp));
    sharpen::SyncPrintf("New Conn:%s:%u->%s:%u\n",remoteIp,remote.GetAddrPtr(),localIp,local.GetPort());
    return sharpen::HostPipelineResult::Continue;
}