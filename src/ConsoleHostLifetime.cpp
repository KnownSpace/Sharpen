#include <sharpen/ConsoleHostLifetime.hpp>

#include <csignal>

sharpen::ConsoleHostLifetime::ConsoleHostLifetime()
    : Self{sharpen::GetLocalScheduler(), sharpen::GetLocalLoopGroup()}
{
}

sharpen::ConsoleHostLifetime::ConsoleHostLifetime(sharpen::IFiberScheduler &scheduler,
                                                  sharpen::IEventLoopGroup &loopGroup)
    : scheduler_(&scheduler)
    , loopGroup_(&loopGroup)
    , host_(nullptr)
{
}

sharpen::ConsoleHostLifetime::ConsoleHostLifetime(Self &&other) noexcept
    : scheduler_(other.scheduler_)
    , loopGroup_(other.loopGroup_)
    , host_(other.host_)
{
    other.scheduler_ = nullptr;
    other.loopGroup_ = nullptr;
    other.host_ = nullptr;
}

sharpen::ConsoleHostLifetime &sharpen::ConsoleHostLifetime::operator=(Self &&other) noexcept
{
    if (this != std::addressof(other))
    {
        this->scheduler_ = other.scheduler_;
        this->loopGroup_ = other.loopGroup_;
        this->host_ = other.host_;
        other.scheduler_ = nullptr;
        other.loopGroup_ = nullptr;
        other.host_ = nullptr;
    }
    return *this;
}

void sharpen::ConsoleHostLifetime::Bind(sharpen::IHost &host)
{
    this->host_ = &host;
}

void sharpen::ConsoleHostLifetime::WaitForSignal(sharpen::SignalChannelPtr channel)
{
    sharpen::SignalBuffer sigs{1};
    try
    {
        std::size_t size{channel->ReadAsync(sigs)};
        (void)size;
    }
    catch (const std::bad_alloc &fatal)
    {
        std::terminate();
        (void)fatal;
    }
    catch (const std::system_error &ignore)
    {
        sharpen::ErrorCode code{sharpen::GetErrorCode(ignore)};
        if (sharpen::IsFatalError(code))
        {
            std::terminate();
        }
        (void)ignore;
    }
    this->host_->Stop();
    this->host_ = nullptr;
}

void sharpen::ConsoleHostLifetime::Run()
{
    if (!this->host_)
    {
        throw std::logic_error{"should bind to a host first"};
    }
    std::int32_t sig{SIGINT};
    sharpen::SignalChannelPtr channel{sharpen::OpenSignalChannel(&sig, 1)};
    channel->Register(*this->loopGroup_);
    this->scheduler_->Launch(&Self::WaitForSignal, this, std::move(channel));
    this->host_->Run();
}
