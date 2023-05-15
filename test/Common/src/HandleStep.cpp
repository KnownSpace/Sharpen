#include <HandleStep.hpp>

#include <sharpen/IpEndPoint.hpp>
#include <sharpen/DebugTools.hpp>

HandleStep::HandleStep(std::uint32_t magic, std::function<void(sharpen::INetStreamChannel*,sharpen::Mail)> handler)
    : factory_()
    , handler_(std::move(handler)) {
    sharpen::IMailParserFactory *factory{new (std::nothrow)
                                             sharpen::GenericMailParserFactory{magic}};
    if (!factory) {
        throw std::bad_alloc{};
    }
    this->factory_.reset(factory);
}

HandleStep &HandleStep::operator=(Self &&other) noexcept {
    if (this != std::addressof(other)) {
        this->factory_ = std::move(other.factory_);
        this->handler_ = std::move(other.handler_);
    }
    return *this;
}

sharpen::HostPipelineResult HandleStep::Consume(sharpen::INetStreamChannel &channel,
                                                const std::atomic_bool &active) noexcept {
    sharpen::IpEndPoint remote;
    channel.GetRemoteEndPoint(remote);
    char remoteIp[25] = {0};
    remote.GetAddrString(remoteIp, sizeof(remoteIp));
    std::unique_ptr<sharpen::IMailParser> parser{this->factory_->Produce()};
    channel.SetKeepAlive(true);
    sharpen::ByteBuffer buf{4096};
    std::size_t size{channel.ReadAsync(buf)};
    while (size != 0) {
        parser->Parse(buf.GetSlice(0, size));
        while (parser->Completed()) {
            sharpen::SyncPrintf("Receive Mail from %s:%u\n",remoteIp,remote.GetPort());
            sharpen::Mail mail{parser->PopCompletedMail()};
            if (this->handler_) {
                this->handler_(&channel,std::move(mail));
            }
        }
        size = channel.ReadAsync(buf);
    }
    return sharpen::HostPipelineResult::Broken;
}