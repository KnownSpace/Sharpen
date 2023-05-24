#include <common/BatchHandler.hpp>

#include <sharpen/DebugTools.hpp>

BatchHandler::BatchHandler(std::uint32_t batch, std::function<void(MailTask *)> handler) noexcept
    : lock_()
    , tasks_()
    , batch_(batch)
    , handler_(std::move(handler)) {
    assert(this->batch_ != 0);
}

void BatchHandler::operator()(sharpen::INetStreamChannel *channel, sharpen::Mail mail) noexcept {
    assert(channel != nullptr);
    {
        std::unique_lock<sharpen::AsyncMutex> lock{this->lock_};
        this->tasks_.emplace_back(*channel,std::move(mail));
        if (this->tasks_.size() == this->batch_) {
            sharpen::SyncPuts("Start handle tasks");
            for(auto begin = this->tasks_.begin(),end = this->tasks_.end(); begin != end; ++begin)
            {
                if(this->handler_) {
                    this->handler_(&(*begin));
                }   
            }
            this->tasks_.clear();
        }
    }
}