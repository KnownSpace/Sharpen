#include <common/RaftStep.hpp>

#include <sharpen/DebugTools.hpp>
#include <sharpen/IpEndPoint.hpp>

RaftStep::RaftStep(std::uint32_t magicNumber, std::shared_ptr<sharpen::IConsensus> raft) noexcept
    : factory_(nullptr)
    , raft_(std::move(raft)) {
    this->factory_.reset(new (std::nothrow) sharpen::GenericMailParserFactory{magicNumber});
    if (!this->factory_) {
        std::terminate();
    }
}

sharpen::HostPipelineResult RaftStep::Consume(sharpen::INetStreamChannel &channel,
                                              const std::atomic_bool &active) noexcept {
    sharpen::IpEndPoint remote;
    channel.GetRemoteEndPoint(remote);
    char remoteIp[25] = {0};
    remote.GetAddrString(remoteIp, sizeof(remoteIp));
    std::unique_ptr<sharpen::IMailParser> parser{this->factory_->Produce()};
    channel.SetKeepAlive(true);
    sharpen::ByteBuffer buf{4096};
    std::size_t size{channel.ReadAsync(buf)};
    auto id{remote.GetActorId()};
    while (size != 0) {
        parser->Parse(buf.GetSlice(0, size));
        while (parser->Completed()) {
            sharpen::SyncPrintf("Receive Mail from %s:%u\n", remoteIp, remote.GetPort());
            sharpen::Mail mail{parser->PopCompletedMail()};
            mail = this->raft_->GenerateResponse(mail);
            channel.WriteAsync(mail.Header());
            if (!mail.Content().Empty()) {
                channel.WriteAsync(mail.Content());
            }
            sharpen::SyncPrintf("Reply to %s:%u Header %zu Content %zu\n",
                                remoteIp,
                                remote.GetPort(),
                                mail.Header().GetSize(),
                                mail.Content().GetSize());
        }
        size = channel.ReadAsync(buf);
    }
    return sharpen::HostPipelineResult::Broken;
}